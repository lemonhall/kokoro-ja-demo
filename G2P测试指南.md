# G2P 准确度对比测试指南

## 📋 目标

对比 **Python (MeCab + Misaki)** 和 **Kotlin (Kuromoji + OpenJTalkG2P)** 的音素输出准确度。

---

## 🚀 测试步骤

### 步骤 1：重新编译并安装 Android 应用

```bash
cd e:\development\kokoro-ja-demo
.\gradlew :app:installDebug --no-daemon
```

> 已添加 G2P 测试日志输出

---

### 步骤 2：准备 adb 日志监控

在一个新的 PowerShell 窗口中运行：

```powershell
adb logcat -s System.out:I | Select-String "G2P_TEST"
```

> 这将实时显示 G2P 转换结果

---

### 步骤 3：在 Android 应用中依次输入测试句子

打开 APP，在**自定义文本输入框**中依次输入以下句子：

#### 📝 测试句子列表

1. **简单问候**
   ```
   こんにちは、私はレモンと申します。
   ```

2. **包含汉字的句子**
   ```
   今日は良い天気ですね。
   ```

3. **地名 + 名词**
   ```
   東京大学からの留学生です。
   ```

4. **长句子**
   ```
   彼女は日本語を勉強していますが、まだ上手ではありません。
   ```

5. **包含促音、长音**
   ```
   学校へ行って、友達と遊びました。
   ```

---

### 步骤 4：收集 Kotlin 输出

从 adb 日志中复制 Kotlin 的音素输出，格式如：

```
G2P_TEST: こんにちは、私はレモンと申します。 -> koNnit ɕiβaβatɕiβaɾemoNtomooɕimasɯ
```

---

### 步骤 5：运行 Python 对比脚本

```bash
uv run python test_g2p_comparison.py
```

Python 脚本会：
1. 自动运行 Python 版本的 G2P
2. 显示 Python 的音素输出
3. 提示你填充 Kotlin 的输出
4. 计算相似度

---

### 步骤 6：更新对比脚本

将步骤 4 收集的 Kotlin 输出填入 `test_g2p_comparison.py` 的第 145 行附近：

```python
# 获取 Kotlin 版本的音素（需要手动填充）
KOTLIN_RESULTS = {
    1: "koNnitɕiβaβatɕiβaɾemoNtomooɕimasɯ",  # 从日志复制
    2: "kʲoːβajoi teNkidesɯne",               # 从日志复制
    3: "tooкʲooдаiɡакокаɾаnoɾʲuuɡакɯseidesɯ", # 从日志复制
    4: "...",                                   # 从日志复制
    5: "...",                                   # 从日志复制
}

def get_kotlin_phonemes(sentence_id: int) -> str:
    return KOTLIN_RESULTS.get(sentence_id, "待填充")
```

---

### 步骤 7：重新运行对比脚本

```bash
uv run python test_g2p_comparison.py
```

现在会显示完整的对比结果！

---

## 📊 预期输出示例

```
================================================================================
                          G2P 准确度对比测试
================================================================================

================================================================================
测试 #1: 问候
================================================================================
📝 原文: こんにちは、私はレモンと申します。
🌐 译文: 你好，我叫柠檬。

🐍 Python (MeCab + Misaki):
   koNnitɕiβaβatɕiβaɾemoNtomooɕimasɯ

🤖 Kotlin (Kuromoji + OpenJTalk):
   koNnitɕiβaβatɕiβaɾemoNtomooɕimasɯ

📊 对比结果: ✅ 完全一致

================================================================================
测试 #2: 日常对话
================================================================================
📝 原文: 今日は良い天气ですね。
🌐 译文: 今天天气真好啊。

🐍 Python (MeCab + Misaki):
   kʲoːβajoi teNkidesɯne

🤖 Kotlin (Kuromoji + OpenJTalk):
   kʲoːβajoi teNkidesɯne

📊 对比结果: ✅ 完全一致

...

================================================================================
                              📊 测试汇总
================================================================================

总测试数: 5
已完成: 5
平均相似度: 95.2%
完全匹配: 4/5
```

---

## 🎯 成功标准

- **✅ 优秀**：平均相似度 ≥ 95%
- **⚠️ 良好**：平均相似度 85-95%
- **❌ 需要改进**：平均相似度 < 85%

---

## 📝 注意事项

1. **确保手机已连接**：`adb devices` 能看到设备
2. **日志过滤**：只看 `G2P_TEST` 相关的日志
3. **字符编码**：复制日志时注意 UTF-8 编码
4. **音素符号**：注意 IPA 符号（ɕ, ɸ, ɡ 等）是否正确

---

## 🔧 故障排除

### 问题 1：adb 命令不存在
```bash
# 确保 Android SDK platform-tools 在 PATH 中
# 或使用完整路径
C:\Users\YourName\AppData\Local\Android\Sdk\platform-tools\adb.exe logcat
```

### 问题 2：看不到日志
```bash
# 清空日志后重试
adb logcat -c
adb logcat -s System.out:I | Select-String "G2P_TEST"
```

### 问题 3：APP 崩溃
```bash
# 查看完整日志
adb logcat *:E
```

---

## 📈 下一步

如果准确度达到 **95%+**，说明：
- ✅ Kuromoji + OpenJTalkG2P 方案成功
- ✅ 可以支持任意日文输入
- ✅ 项目**大功告成**！🎉

如果准确度低于 95%，需要：
- 🔍 分析差异原因
- 🛠️ 调整 OpenJTalkG2P 的规则
- 📝 记录问题和解决方案

---

**祝测试顺利！** 🚀
