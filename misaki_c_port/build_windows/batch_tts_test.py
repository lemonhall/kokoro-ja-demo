"""
批量TTS对比测试 - 优化版
C版本 vs Python版本 G2P + ONNX TTS
常驻内存，批量生成
"""

import ctypes
from pathlib import Path
import sys
import os
import numpy as np

# 添加项目根目录到 sys.path
project_root = Path(__file__).parent.parent.parent
if str(project_root) not in sys.path:
    sys.path.insert(0, str(project_root))

# 导入依赖
try:
    from misaki.zh import ZHG2P
    from kokoro import KPipeline
    import soundfile as sf
    import onnxruntime as ort
    DEPS_OK = True
except ImportError as e:
    print(f"❌ 依赖缺失: {e}")
    DEPS_OK = False


class GlobalTTSEngine:
    """全局TTS引擎 - 常驻内存"""
    
    def __init__(self):
        print("🚀 初始化全局TTS引擎...")
        
        # 1. 初始化C版本G2P
        dll_path = Path(__file__).parent / "libmisaki.dll"
        os.chdir(dll_path.parent)
        self.c_lib = ctypes.CDLL(str(dll_path.absolute()))
        
        self.c_lib.misaki_init.argtypes = [ctypes.c_char_p]
        self.c_lib.misaki_init.restype = ctypes.c_int
        self.c_lib.misaki_text_to_phonemes_lang.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
        self.c_lib.misaki_text_to_phonemes_lang.restype = ctypes.c_int
        
        result = self.c_lib.misaki_init(b"extracted_data")
        if result != 0:
            raise RuntimeError("C Misaki 初始化失败")
        print("   ✅ C版本G2P已加载")
        
        # 2. 初始化Python版本G2P
        self.py_g2p = ZHG2P()
        print("   ✅ Python版本G2P已加载")
        
        # 3. 初始化ONNX模型
        onnx_path = project_root / "kokoro_latest.onnx"
        if not onnx_path.exists():
            raise FileNotFoundError(f"模型文件不存在: {onnx_path}")
        
        self.onnx_session = ort.InferenceSession(str(onnx_path))
        print("   ✅ ONNX模型已加载")
        
        # 4. 初始化Kokoro Pipeline（获取vocab和voice）
        self.pipeline = KPipeline(lang_code='z')
        self.vocab = self.pipeline.model.vocab
        
        # 5. 预加载语音嵌入
        self.voice_name = 'zf_xiaoxiao'
        self.voices_tensor = self.pipeline.load_voice(self.voice_name)
        print(f"   ✅ 语音嵌入已加载 ({self.voice_name})")
        
        print("✅ 全局TTS引擎初始化完成\n")
    
    def c_g2p(self, text):
        """C版本G2P"""
        output_buffer = ctypes.create_string_buffer(2048)
        result = self.c_lib.misaki_text_to_phonemes_lang(
            text.encode('utf-8'), 
            b'zh',  # 强制中文
            output_buffer, 
            len(output_buffer)
        )
        return output_buffer.value.decode('utf-8') if result == 0 else None
    
    def py_g2p_convert(self, text):
        """Python版本G2P"""
        phonemes, _ = self.py_g2p(text)
        return phonemes
    
    def synthesize(self, phonemes):
        """音素转音频（复用ONNX session）"""
        # 音素转 input_ids
        input_ids = list(filter(lambda i: i is not None, map(lambda p: self.vocab.get(p), phonemes)))
        input_ids_with_special = [0, *input_ids, 0]
        
        # 动态选择语音帧
        frame_index = min(len(input_ids) - 1, self.voices_tensor.shape[0] - 1)
        ref_s = self.voices_tensor[frame_index, 0, :].unsqueeze(0).numpy()
        
        # ONNX推理
        inputs = {
            'input_ids': np.array([input_ids_with_special], dtype=np.int64),
            'ref_s': ref_s,
            'speed': np.array(1.0, dtype=np.float64)
        }
        
        outputs = self.onnx_session.run(None, inputs)
        waveform = outputs[0]
        
        # 归一化
        if np.max(np.abs(waveform)) > 1.0:
            waveform = waveform / np.max(np.abs(waveform)) * 0.95
        
        return waveform
    
    def cleanup(self):
        """清理资源"""
        if hasattr(self, 'c_lib'):
            self.c_lib.misaki_cleanup()


# 测试语料 - 系统性测试方案（扩展版 50+ 用例）
TEST_SENTENCES = [
    # ===== 1. 声母"口音"探测集 (专门抓 ʂ/ʈʂ 等怪音) =====
    "知道吃饭的时候，市场上正在杀猪",  # 大量 zh, ch, sh, r
    "四是四，十是十，十四是十四",      # 经典绕口令，精准打击
    "这个人真的值得注意",             # 聚焦 zhi, chi 和 r
    "吃葡萄不吐葡萄皮",               # 混合挑战
    "出租车司机在十字路口",           # ch, z, sh 混合
    "制作者制止了这次事故",           # zh, ch, sh 密集
    "诗人写诗抒发真挚情感",           # sh, zh 交替
    "日本人热爱樱花如痴如醉",          # r 声母高频
    
    # ===== 2. 韵母"方言"探测集 (专门抓 en/eng, in/ing 不分) =====
    "人民认为认真的人能成功",         # 大量 en, eng, ren, neng
    "心情新颖，经营音乐",             # in, ing 大混战
    "长江长城，黄山黄河",             # ang, eng, uan 组合
    "根本问题，认真解决",             # en, eng 交替出现
    "今天金融形势稳定",               # in, ing, en 混合
    "拼命争取胜利成功",               # ing, en 密集
    "心灵深处感受温暖",               # in, en 对比
    "更正本月工程进度",               # eng, en 交错
    "清晨听音乐陶冶情操",             # ing, en 组合
    
    # ===== 3. 音变"生硬"探测集 (测试语流音变自然度) =====
    "我们一起去看电影吧",             # 轻声、儿化、连读
    "这个小孩子跑得很快",             # 子（轻声）、得（轻声）
    "一点儿问题都没有",               # 儿化音检测
    "是不是应该好好的考虑一下",        # 重叠词、轻声
    "他说的话我听不懂",               # 的、不 轻声
    "桌子上摆着苹果",                 # 子、着 轻声
    "你们俩来这儿玩儿",               # 多重儿化
    "差不多了吧",                     # 轻声连用
    "孩子们高高兴兴地唱歌",           # 重叠词、地 轻声
    
    # ===== 4. 声调"怪异"探测集 (测试声调连续变调) =====
    "我可以理解你的想法",             # 三声变调 (我-可)
    "一起一起一起走",                 # 相同字不同语调
    "这个东西好不好吃",               # 轻声、疑问语调
    "你好吗好不好",                   # 好字多重声调
    "我很好你也很好",                 # 三声变调序列
    "买水果买苹果买香蕉",             # 买字重复测试
    "老李老王老张都来了",             # 老字声调变化
    "不对不是不会",                   # 不字变调
    
    # ===== 5. 鼻音对比组 (前鼻音 n vs 后鼻音 ng) =====
    "安然间断，反感长江",             # an vs ang
    "银行运营英勇",                   # in vs ing
    "本能崩溃，蓬勃分明",             # en vs eng
    "温暖翁翁嗡嗡",                   # un vs ong
    
    # ===== 6. 平翘舌对比组 (z/c/s vs zh/ch/sh) =====
    "祖师组织措施资助",               # 平翘舌混合
    "思想深刻此次师资",               # s vs sh 对比
    "次次尝试差错猜测",               # c vs ch 密集
    "杂志早早自助遮蔽",               # z vs zh 序列
    
    # ===== 7. 复杂韵母组 (iou, uei, uen 等简化规则) =====
    "走走看看有有无无",               # iou, ou 对比
    "回归微微味味",                   # uei, ei 测试
    "问问文文云云",                   # uen, un 变化
    
    # ===== 8. 原有测试用例（保留基础覆盖） =====
    "今天天气真不错",
    "明天我们去爬山",
    "北京市朝阳区",
    "中国人民解放军",
    "音乐让人心情愉悦",
    "新颖的设计理念",
    "经营一家餐厅",
    "时间过得真快啊",
    "太阳从东方升起",
    "月亮在天上挂着",
    "春天来了万物复苏",
    
    # ===== 9. 多音字陷阱组 =====
    "银行行长要去行走",               # 行 (háng, xíng)
    "重要的重量很重",                 # 重 (zhòng, chóng)
    "长江很长",                       # 长 (cháng)
    "当天应当马上出发",               # 当 (dāng)
    "数学老师数不清人数",             # 数 (shù, shǔ)
    "还有还钱的事",                   # 还 (hái, huán)
    "差不多差了一点",                 # 差 (chà, chā)
    
    # ===== 10. 特殊音节组 (ü 韵母) =====
    "女儿去旅游玩绿色",               # nü, lü, qu
    "虚心学习需要努力",               # xu, qu, nu
    "鱼与玉遇雨",                     # yu 系列
    "聚集菊花局部曲目",               # ju, qu 测试
    
    # ===== 11. 轻声极限测试 =====
    "桌子上的东西是我的",             # 多重轻声
    "我们的朋友们都来了",             # 们、的、了 轻声
    "你看看他们在干什么",             # 看看、什么 轻声
    
    # ===== 12. 实用场景测试 =====
    "欢迎使用智能语音助手",           # 客服场景
    "导航已为您规划最优路线",          # 导航场景
    "请输入您的密码",                 # 安全场景
    "感谢您的支持与理解",             # 感谢场景
    "系统升级维护中",                 # 系统通知
    
    # ===== 13. 数字与量词组合 =====
    "一百二十三个苹果",               # 数字读法
    "第一名第二名第三名",             # 序数词
    "两千零二十四年",                 # 年份读法
    "三点五公斤",                     # 小数点读法
    "百分之九十五",                   # 百分比
    "零下十五度",                     # 负数温度
    "一万五千块钱",                   # 货币数额
    
    # ===== 14. 叠音词与拟声词 =====
    "哗啦哗啦下雨了",                 # 拟声词
    "咕咚咕咚喝水",                   # 叠音拟声
    "叮叮当当响个不停",               # 双叠音
    "淅淅沥沥的小雨",                 # 叠韵
    "滴滴答答的钟声",                 # 叠音序列
    "噼里啪啦炒菜声",                 # 多音节拟声
    
    # ===== 15. 方言特色词汇 =====
    "这个东西挺好的嘞",               # 语气词
    "你晓得不晓得",                   # 西南方言
    "搞啥子嘛",                       # 四川话
    "木有问题",                       # 网络用语
    "酱紫就可以了",                   # 谐音词
    
    # ===== 16. 外来词与混合词 =====
    "这个思佩思很好看",               # space 音译
    "打卡网红咖啡店",                 # 外来词+本土词
    "他很佛系",                       # 流行词汇
    "这个巴士站",                     # bus 音译
    "披萨和汉堡",                     # 食品外来词
    
    # ===== 17. 专有名词与地名 =====
    "从上海到杭州",                   # 城市名
    "黄河长江珠江",                   # 河流名
    "泰山华山衡山",                   # 山名
    "东北西北东南西南",               # 方位词
    "广东广西福建江西",               # 省份名
    
    # ===== 18. 成语与四字词组 =====
    "一心一意全力以赴",               # 成语连用
    "风和日丽春暖花开",               # 自然成语
    "马到成功旗开得胜",               # 吉祥成语
    "千钧一发刻不容缓",               # 紧急成语
    "循序渐进脚踏实地",               # 学习成语
    "井井有条有条不紊",               # 秩序成语
    
    # ===== 19. 情感表达与语气 =====
    "太棒了真的太棒了",               # 强烈情感
    "哎呀不好意思啊",                 # 道歉语气
    "哇塞好厉害呀",                   # 惊叹语气
    "唉真是没办法",                   # 无奈语气
    "哼才不要呢",                     # 拒绝语气
    "嗯好的明白了",                   # 确认语气
    
    # ===== 20. 连词与虚词密集 =====
    "因为所以但是可是",               # 连词串
    "虽然然而不过而且",               # 转折词
    "如果那么就是于是",               # 假设词
    "无论还是或者以及",               # 选择词
    "不仅而且并且以及",               # 递进词
    
    # ===== 21. 时间与日期表达 =====
    "今天明天后天大后天",             # 相对时间
    "昨天前天大前天",                 # 过去时间
    "早上中午下午晚上",               # 时段
    "春夏秋冬四季",                   # 季节
    "一月二月三月四月",               # 月份
    
    # ===== 22. 动词时态与语态 =====
    "正在做着要去做",                 # 进行时+完成时+将来时
    "吃了喝了睡了",                   # 完成态
    "看看听听说说",                   # 尝试态
    "跑过来走过去",                   # 趋向动词
    "打开关上放下",                   # 结果补语
    
    # ===== 23. 形容词叠加与比较 =====
    "红红的绿绿的",                   # 叠加形容词
    "又大又圆又好看",                 # 并列形容词
    "越来越好越来越快",               # 渐进比较
    "最好最快最强",                   # 最高级
    "更高更远更强",                   # 比较级
    
    # ===== 24. 疑问句与反问句 =====
    "你去哪里干什么",                 # 疑问词串
    "为什么怎么样",                   # 原因疑问
    "是吗真的吗可能吗",               # 确认疑问
    "谁知道呢谁说的",                 # 反问句
    "难道不是吗",                     # 反问加强
    
    # ===== 25. 祈使句与命令句 =====
    "快点快点快一点",                 # 催促语气
    "别动不要动",                     # 禁止命令
    "请坐请喝茶",                     # 礼貌祈使
    "让开让一让",                     # 要求语气
    "小心注意安全",                   # 提醒语气
    
    # ===== 26. 长句与复杂句式 =====
    "如果明天天气好的话我们就去公园散步",     # 条件复句
    "虽然很累但是还是坚持完成了任务",         # 转折复句
    "不仅学习好而且身体也很健康",             # 递进复句
    "因为下雨所以比赛取消了",                 # 因果复句
    "只要努力就一定能成功",                   # 假设复句
    
    # ===== 27. 口语化表达 =====
    "嗯哼啊哈哦",                     # 语气词连用
    "这个那个这样那样",               # 指示代词
    "怎么说呢就是那种感觉",           # 口语填充
    "对对对没错没错",                 # 肯定重复
    "算了算了不说了",                 # 放弃语气
    
    # ===== 28. 书面语与正式表达 =====
    "兹定于某月某日召开会议",         # 公文用语
    "鉴于上述情况特此通知",           # 正式通知
    "根据相关规定执行",               # 法律用语
    "综上所述得出结论",               # 学术用语
    "经研究决定批准实施",             # 行政用语
]


def batch_test(engine, output_dir="batch_output"):
    """批量测试"""
    output_path = Path(output_dir)
    output_path.mkdir(exist_ok=True)
    
    print("="*70)
    print("🎯 批量TTS对比测试")
    print("="*70)
    print(f"📊 测试语料: {len(TEST_SENTENCES)} 句\n")
    
    for idx, text in enumerate(TEST_SENTENCES, 1):
        print(f"[{idx}/{len(TEST_SENTENCES)}] {text}")
        
        # 1. C版本G2P
        c_phonemes = engine.c_g2p(text)
        
        # 2. Python版本G2P
        py_phonemes = engine.py_g2p_convert(text)
        
        # 3. 对比音素
        match = "✅" if c_phonemes == py_phonemes else "❌"
        print(f"   C:  {c_phonemes}")
        print(f"   Py: {py_phonemes}")
        print(f"   {match} {'相同' if match == '✅' else '不同'}")
        
        # 4. 生成音频
        c_audio = engine.synthesize(c_phonemes)
        py_audio = engine.synthesize(py_phonemes)
        
        # 5. 保存音频
        c_filename = output_path / f"{idx:03d}_c_{text[:10]}.wav"
        py_filename = output_path / f"{idx:03d}_py_{text[:10]}.wav"
        
        sf.write(str(c_filename), c_audio, 24000)
        sf.write(str(py_filename), py_audio, 24000)
        
        # 6. 保存音素日志（每个用例一个独立文件）
        log_filename = output_path / f"{idx:03d}_phonemes.txt"
        with open(log_filename, 'w', encoding='utf-8') as log:
            log.write(f"输入文本: {text}\n")
            log.write(f"C版本音素: {c_phonemes}\n")
            log.write(f"Python版本音素: {py_phonemes}\n")
        
        print(f"   💾 已保存: {c_filename.name} / {py_filename.name}")
        print(f"   📝 音素日志: {log_filename.name}\n")
    
    print("="*70)
    print(f"✅ 批量测试完成！")
    print(f"📁 输出目录: {output_path.absolute()}")
    print(f"🎵 共生成: {len(TEST_SENTENCES) * 2} 个音频文件")
    print(f"📝 共生成: {len(TEST_SENTENCES)} 个音素日志文件")
    print("="*70)


def main():
    if not DEPS_OK:
        print("❌ 请先安装依赖")
        return
    
    # 初始化全局引擎（常驻内存）
    engine = GlobalTTSEngine()
    
    try:
        # 批量测试
        batch_test(engine)
    finally:
        # 清理
        engine.cleanup()
        print("\n✅ 资源已清理")


if __name__ == "__main__":
    main()
