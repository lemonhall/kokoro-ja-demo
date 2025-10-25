# 更新日志

## [未发布] - 2025-10-25

### ✨ 新增功能

#### 🌐 多语言支持
- **中文语音合成** 🇨🇳
  - 基于 pypinyin 的拼音转换系统
  - 20,902 个汉字覆盖
  - ChinesePinyinToIPA 音素映射（21声母 + 40韵母 + 5声调）
  - zf_xiaoxiao 中文女声音色

- **英文语音合成** 🇺🇸
  - CMUdict 完整发音词典
  - 126,052 个英文单词支持
  - ARPAbet → IPA 音素转换
  - 未知单词自动回退到字母拼读

- **智能语言检测**
  - 基于 Unicode 字符范围的语言识别
  - 支持中文、日文、英文自动检测
  - 准确率 > 95%

- **混合语言处理**
  - 自动语言分段：逐字检测语言变化
  - 无缝音素拼接：各段独立 G2P 处理
  - 支持任意中日英混合文本

### 🐛 Bug 修复

- **修复中文音素分隔问题**
  - 问题：ChineseG2PSystem 输出的音素之间没有空格
  - 影响：混合语言拼接时音素粘连，模型无法正确识别
  - 修复：改用 `joinToString(" ")` 添加空格分隔

### 📦 新增文件

#### Kotlin 源码
- `app/src/main/java/.../LanguageDetector.kt` - 语言自动检测器
- `app/src/main/java/.../UnifiedG2PSystem.kt` - 统一 G2P 入口
- `app/src/main/java/.../ChineseG2PSystem.kt` - 中文 G2P 系统
- `app/src/main/java/.../ChinesePinyinToIPA.kt` - 拼音到IPA映射
- `app/src/main/java/.../EnglishG2PSystem.kt` - 英文 G2P 系统

#### 资源文件
- `app/src/main/assets/pinyin_dict.json` - 中文拼音字典（267KB）
- `app/src/main/assets/english_dict.json` - 英文发音字典（3.3MB）
- `app/src/main/assets/zf_xiaoxiao.bin` - 中文女声嵌入（522KB）

#### Python 脚本
- `generate_pinyin_dict.py` - 生成中文拼音字典
- `generate_english_dict.py` - 生成英文发音字典
- `test_mixed_language.py` - 混合语言测试脚本

#### 测试文件
- `app/src/test/java/.../LanguageDetectorTest.kt` - 语言检测测试
- `app/src/test/java/.../MixedLanguageTest.kt` - 混合语言测试
- `app/src/test/java/.../MixedLanguageFlowTest.kt` - 完整流程测试

### 📝 文档更新

- 更新 `README.md`：添加多语言支持说明
- 整理根目录：移动过程文档到 `docs/progress/`
- 添加多语言处理流程示例

### 🔧 技术细节

#### 中文 G2P 实现原则
- ✅ 无外部依赖：Python 导出字典 → Kotlin 加载 JSON
- ✅ 轻量级：267KB 字典文件
- ✅ 快速落地：pypinyin TONE3 格式（如 ni3 hao3）

#### 英文 G2P 实现原则
- ✅ 完整词典：CMUdict 126,052 单词
- ✅ 无外部依赖：使用 `pronouncing` 库导出
- ✅ 回退机制：未知单词自动字母拼读

#### 混合语言处理策略
- 方案：分段处理（推荐）
- 逻辑：逐字检测 → 语言切换时分段 → 各段独立 G2P → 拼接
- 优点：准确度高，各语言独立处理

### ⚠️ 已知问题

- 中文 G2P 准确率约 85%，勉强可用
- 英文未知单词回退到字母拼读，发音可能不自然
- 日文 G2P 准确率 82.6%，仍有提升空间

---

## [阶段3] - 2025-10-24

### ✨ 核心突破

- **音质修复** 🎉
  - 发现语音嵌入只用了第一帧的问题
  - 实现动态帧选择：`embedding[phonemeLength - 1]`
  - 音质与 PyTorch 原版完全一致

- **性能优化**
  - 升级 ONNX Runtime 至 1.23.1
  - RTF 从 1.62x 提升至 0.70x（提升 57%）
  - 满足实时语音合成要求

### 📦 新增功能

- **完整 G2P 系统**
  - Kuromoji 分词
  - OpenJTalk 规则移植
  - 支持汉字输入（准确率 82.6%）

---

## [阶段2] - 2025-10-23

### 🐛 问题分析

- 发现 ONNX 音质低沉浑厚的问题
- 分析语音嵌入数据结构
- 验证 510 帧嵌入的必要性

---

## [阶段1] - 2025-10-22

### ✨ 初步成果

- 成功导出 ONNX 模型
- Android 集成 ONNX Runtime
- 基本的假名 G2P 系统

### ⚠️ 遗留问题

- ONNX 推理速度慢（RTF 1.62x）
- 音质与 PyTorch 不一致
- 只支持假名输入

---

## [阶段0] - 2025-10-21

### 🎯 项目启动

- 研究 Kokoro TTS 引擎
- PyTorch 版本验证
- 技术路线规划
