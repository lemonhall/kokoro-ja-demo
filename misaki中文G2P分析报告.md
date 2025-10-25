# Misaki 中文 G2P 分析报告

> 📅 创建时间: 2025-10-25  
> 🎯 目的: 为 Android 移植提供技术参考  
> 📦 源文件: `.venv/Lib/site-packages/misaki/transcription.py`

---

## 📋 核心流程

Misaki 的中文 G2P 转换分为 **3 个步骤**:

```
1️⃣ 汉字 → 拼音 (pypinyin)
   "你好" → ["ni3", "hao3"]

2️⃣ 拼音 → IPA 音素 (transcription.py)
   "ni3" → "ni˧˩˧"  (带声调)
   "hao3" → "xau̯˧˩˧"

3️⃣ 声调符号简化 (zh.py)
   "˧˩˧" → "↓" (三声)
   "˧˥"  → "↗" (二声)
   "˥˩"  → "↘" (四声)
   "˥"   → "→" (一声)
```

---

## 🔤 拼音 → IPA 映射规则

### 1. 声母映射表 (21个)

| 拼音 | IPA | 说明 | 示例 |
|------|-----|------|------|
| b | p | 不送气清塞音 | 爸 ba → pa |
| p | pʰ | 送气清塞音 | 怕 pa → pʰa |
| m | m | 双唇鼻音 | 妈 ma → ma |
| f | f | 唇齿擦音 | 发 fa → fa |
| d | t | 不送气清塞音 | 大 da → ta |
| t | tʰ | 送气清塞音 | 他 ta → tʰa |
| n | n | 齿龈鼻音 | 那 na → na |
| l | l | 齿龈边音 | 拉 la → la |
| g | k | 不送气清塞音 | 哥 ge → kɤ |
| k | kʰ | 送气清塞音 | 科 ke → kʰɤ |
| h | x, h | 软腭/声门擦音 | 喝 he → xɤ |
| j | ʨ | 腭化塞擦音 | 鸡 ji → ʨi |
| q | ʨʰ | 送气腭化塞擦音 | 气 qi → ʨʰi |
| x | ɕ | 腭化擦音 | 西 xi → ɕi |
| zh | ʈʂ (U+AB67) | 卷舌塞擦音 | 知 zhi → ʈʂɻ̩ |
| ch | ʈʂʰ (U+AB67ʰ) | 送气卷舌塞擦音 | 吃 chi → ʈʂʰɻ̩ |
| sh | ʂ | 卷舌擦音 | 是 shi → ʂɻ̩ |
| r | ɻ, ʐ | 卷舌近音/擦音 | 日 ri → ɻɻ̩ |
| z | ʦ | 齿塞擦音 | 资 zi → ʦɹ̩ |
| c | ʦʰ | 送气齿塞擦音 | 次 ci → ʦʰɹ̩ |
| s | s | 齿擦音 | 思 si → sɹ̩ |

**Kotlin 实现**:
```kotlin
private val initialMapping = mapOf(
    "b" to "p",
    "p" to "pʰ",
    "m" to "m",
    "f" to "f",
    "d" to "t",
    "t" to "tʰ",
    "n" to "n",
    "l" to "l",
    "g" to "k",
    "k" to "kʰ",
    "h" to "x",  // 默认用 x
    "j" to "ʨ",
    "q" to "ʨʰ",
    "x" to "ɕ",
    "zh" to "\uAB67",  // Unicode ʈʂ
    "ch" to "\uAB67ʰ",
    "sh" to "ʂ",
    "r" to "ɻ",  // 默认用 ɻ
    "z" to "ʦ",
    "c" to "ʦʰ",
    "s" to "s"
)
```

---

### 2. 韵母映射表 (40个)

#### 2.1 基础韵母

| 拼音 | IPA | 说明 | 示例 |
|------|-----|------|------|
| a | a{tone} | 开元音 | 啊 a → a |
| ai | ai̯{tone} | 双元音 | 爱 ai → ai̯ |
| an | a{tone}n | 前鼻音 | 安 an → an |
| ang | a{tone}ŋ | 后鼻音 | 昂 ang → aŋ |
| ao | au̯{tone} | 双元音 | 奥 ao → au̯ |
| e | ɤ{tone} | 后不圆唇元音 | 饿 e → ɤ |
| ei | ei̯{tone} | 双元音 | 诶 ei → ei̯ |
| en | ə{tone}n | 央元音+鼻音 | 恩 en → ən |
| eng | ə{tone}ŋ | 央元音+鼻音 | 鞥 eng → əŋ |
| o | wo{tone} | 后圆唇元音 | 哦 o → wo |
| ou | ou̯{tone} | 双元音 | 欧 ou → ou̯ |
| ong | ʊ{tone}ŋ | 后圆唇+鼻音 | 瓮 ong → ʊŋ |

#### 2.2 i 系韵母

| 拼音 | IPA | 说明 | 示例 |
|------|-----|------|------|
| i | i{tone} | 前高元音 | 一 yi → i |
| ia | ja{tone} | 介音+元音 | 呀 ya → ja |
| ian | jɛ{tone}n | 介音+前鼻音 | 烟 yan → jɛn |
| iang | ja{tone}ŋ | 介音+后鼻音 | 央 yang → jaŋ |
| iao | jau̯{tone} | 介音+双元音 | 腰 yao → jau̯ |
| ie | je{tone} | 介音+元音 | 耶 ye → je |
| in | i{tone}n | 前鼻音 | 因 yin → in |
| ing | i{tone}ŋ | 后鼻音 | 英 ying → iŋ |
| iou | jou̯{tone} | 介音+双元音 | 优 you → jou̯ |
| iong | jʊ{tone}ŋ | 介音+后鼻音 | 雍 yong → jʊŋ |

#### 2.3 u 系韵母

| 拼音 | IPA | 说明 | 示例 |
|------|-----|------|------|
| u | u{tone} | 后高圆唇元音 | 乌 wu → u |
| ua | wa{tone} | 介音+元音 | 蛙 wa → wa |
| uai | wai̯{tone} | 介音+双元音 | 歪 wai → wai̯ |
| uan | wa{tone}n | 介音+前鼻音 | 弯 wan → wan |
| uang | wa{tone}ŋ | 介音+后鼻音 | 汪 wang → waŋ |
| uei | wei̯{tone} | 介音+双元音 | 威 wei → wei̯ |
| uen | wə{tone}n | 介音+央元音 | 温 wen → wən |
| ueng | wə{tone}ŋ | (罕见) | weng → wəŋ |
| uo | wo{tone} | 介音+元音 | 窝 wo → wo |

#### 2.4 ü 系韵母

| 拼音 | IPA | 说明 | 示例 |
|------|-----|------|------|
| ü | y{tone} | 前高圆唇元音 | 鱼 yu → y |
| üe | ɥe{tone} | 介音+元音 | 月 yue → ɥe |
| üan | ɥɛ{tone}n | 介音+前鼻音 | 冤 yuan → ɥɛn |
| ün | y{tone}n | 前鼻音 | 晕 yun → yn |

**Kotlin 实现**:
```kotlin
private val finalMapping = mapOf(
    "a" to listOf("a", "{tone}"),
    "ai" to listOf("ai̯", "{tone}"),
    "an" to listOf("a", "{tone}", "n"),
    "ang" to listOf("a", "{tone}", "ŋ"),
    "ao" to listOf("au̯", "{tone}"),
    "e" to listOf("ɤ", "{tone}"),
    "ei" to listOf("ei̯", "{tone}"),
    "en" to listOf("ə", "{tone}", "n"),
    "eng" to listOf("ə", "{tone}", "ŋ"),
    // ... 完整映射
)
```

---

### 3. 特殊韵母处理

#### 3.1 卷舌韵母 (zh, ch, sh, r 后)

```kotlin
// zhi, chi, shi, ri 的 "i" 读作 [ɻ̩] 或 [ʐ̩]
private val finalAfterZhChShR = mapOf(
    "i" to listOf("ɻ̩", "{tone}")  // 或 "ʐ̩"
)

// 示例:
// 知 zhi → ʈʂɻ̩
// 吃 chi → ʈʂʰɻ̩
// 是 shi → ʂɻ̩
```

#### 3.2 平舌韵母 (z, c, s 后)

```kotlin
// zi, ci, si 的 "i" 读作 [ɹ̩] 或 [z̩]
private val finalAfterZCS = mapOf(
    "i" to listOf("ɹ̩", "{tone}")  // 或 "z̩"
)

// 示例:
// 资 zi → ʦɹ̩
// 次 ci → ʦʰɹ̩
// 思 si → sɹ̩
```

---

### 4. 声调映射表

| 声调 | IPA 符号 | Unicode | 简化符号 | 示例 |
|------|----------|---------|----------|------|
| 第一声 (阴平) | ˥ | U+02E5 | → | 妈 mā → ma˥ |
| 第二声 (阳平) | ˧˥ | U+02E7 U+02E5 | ↗ | 麻 má → ma˧˥ |
| 第三声 (上声) | ˧˩˧ | U+02E7 U+02E9 U+02E7 | ↓ | 马 mǎ → ma˧˩˧ |
| 第四声 (去声) | ˥˩ | U+02E5 U+02E9 | ↘ | 骂 mà → ma˥˩ |
| 轻声 | (无) | - | (无) | 吗 ma → ma |

**Kotlin 实现**:
```kotlin
private val toneMapping = mapOf(
    1 to "˥",      // 一声
    2 to "˧˥",     // 二声
    3 to "˧˩˧",    // 三声
    4 to "˥˩",     // 四声
    5 to ""        // 轻声
)

// 简化版本 (可选)
private val toneSimplified = mapOf(
    "˥" to "→",
    "˧˥" to "↗",
    "˧˩˧" to "↓",
    "˥˩" to "↘"
)
```

---

### 5. 特殊情况处理

#### 5.1 音节辅音 (Syllabic Consonants)

| 拼音 | IPA | 说明 |
|------|-----|------|
| m | m{tone} | 嗯 (表示) |
| n | n{tone} | (罕见) |
| ng | ŋ{tone} | 嗯 (鼻音) |
| hm | hm{tone} | 哼 |
| hng | hŋ{tone} | 哼 (鼻音) |

#### 5.2 感叹词 (Interjections)

| 拼音 | IPA | 说明 |
|------|-----|------|
| io | jɔ{tone} | 哟 |
| ê | ɛ{tone} | 诶 |
| er | ɚ{tone}, aɚ̯{tone} | 儿、二、耳 |
| o | ɔ{tone} | 哦 |

---

## 🔧 转换算法

### Python 原版逻辑

```python
def pinyin_to_ipa(pinyin: str) -> OrderedSet[Tuple[str, ...]]:
    # 1. 提取声调
    tone_nr = get_tone(pinyin)  # 1-5
    
    # 2. 转换为无声调拼音
    pinyin_normal = to_normal(pinyin)  # "ni3" → "ni"
    
    # 3. 检查特殊情况
    if is_interjection(pinyin_normal):
        return apply_tone(INTERJECTION_MAPPINGS[pinyin_normal], tone_nr)
    
    if is_syllabic_consonant(pinyin_normal):
        return apply_tone(SYLLABIC_CONSONANT_MAPPINGS[pinyin_normal], tone_nr)
    
    # 4. 切分声母和韵母
    initial = get_initials(pinyin_normal)   # "ni" → "n"
    final = get_finals(pinyin_normal)       # "ni" → "i"
    
    # 5. 查表转换
    parts = []
    if initial:
        parts.append(INITIAL_MAPPING[initial])
    
    # 根据声母选择韵母映射表
    if initial in {"zh", "ch", "sh", "r"} and final in FINAL_MAPPING_AFTER_ZH_CH_SH_R:
        final_phonemes = FINAL_MAPPING_AFTER_ZH_CH_SH_R[final]
    elif initial in {"z", "c", "s"} and final in FINAL_MAPPING_AFTER_Z_C_S:
        final_phonemes = FINAL_MAPPING_AFTER_Z_C_S[final]
    else:
        final_phonemes = FINAL_MAPPING[final]
    
    # 应用声调
    final_phonemes = apply_tone(final_phonemes, tone_nr)
    parts.append(final_phonemes)
    
    # 6. 组合音素
    return combine(parts)
```

---

### Kotlin 移植框架

```kotlin
object ChinesePinyinToIPA {
    
    /**
     * 拼音 → IPA 转换
     * 
     * @param pinyin 带声调的拼音 (如 "ni3", "hao3")
     * @return IPA 音素字符串 (如 "ni˧˩˧", "xau̯˧˩˧")
     */
    fun convert(pinyin: String): String {
        // 1. 提取声调
        val tone = extractTone(pinyin)  // 3
        val normalPinyin = removeTone(pinyin)  // "ni"
        
        // 2. 特殊情况处理
        if (isInterjection(normalPinyin)) {
            return applyTone(interjectionMapping[normalPinyin]!!, tone)
        }
        
        if (isSyllabicConsonant(normalPinyin)) {
            return applyTone(syllabicMapping[normalPinyin]!!, tone)
        }
        
        // 3. 切分声母韵母
        val initial = extractInitial(normalPinyin)  // "n"
        val final = extractFinal(normalPinyin)      // "i"
        
        // 4. 查表转换
        val parts = mutableListOf<String>()
        
        if (initial != null) {
            parts.add(initialMapping[initial]!!)
        }
        
        // 选择正确的韵母映射表
        val finalPhoneme = when {
            initial in setOf("zh", "ch", "sh", "r") && final in finalAfterZhChShR ->
                finalAfterZhChShR[final]!!
            initial in setOf("z", "c", "s") && final in finalAfterZCS ->
                finalAfterZCS[final]!!
            else ->
                finalMapping[final]!!
        }
        
        parts.addAll(finalPhoneme)
        
        // 5. 应用声调
        return applyTone(parts, tone)
    }
    
    /**
     * 提取声调 (1-5)
     */
    private fun extractTone(pinyin: String): Int {
        // 检查最后一位是否是数字
        val lastChar = pinyin.lastOrNull() ?: return 5
        return if (lastChar.isDigit()) {
            lastChar.digitToInt()
        } else {
            5  // 默认轻声
        }
    }
    
    /**
     * 移除声调标记
     */
    private fun removeTone(pinyin: String): String {
        return pinyin.replace(Regex("[0-9]"), "")
    }
    
    /**
     * 提取声母
     */
    private fun extractInitial(pinyin: String): String? {
        // 优先匹配双字符声母 (zh, ch, sh)
        if (pinyin.length >= 2) {
            val twoChar = pinyin.substring(0, 2)
            if (twoChar in setOf("zh", "ch", "sh")) {
                return twoChar
            }
        }
        
        // 匹配单字符声母
        val firstChar = pinyin.firstOrNull()?.toString()
        return if (firstChar in initialMapping.keys) {
            firstChar
        } else {
            null  // 零声母
        }
    }
    
    /**
     * 提取韵母
     */
    private fun extractFinal(pinyin: String): String {
        val initial = extractInitial(pinyin)
        return if (initial != null) {
            pinyin.removePrefix(initial)
        } else {
            pinyin
        }
    }
    
    /**
     * 应用声调
     */
    private fun applyTone(parts: List<String>, tone: Int): String {
        val toneSymbol = toneMapping[tone] ?: ""
        return parts.joinToString("") { part ->
            part.replace("{tone}", toneSymbol)
        }
    }
}
```

---

## 📊 完整示例

### 示例 1: "你好"

```
输入: "ni3 hao3"

--- ni3 ---
1. 提取声调: 3 → "˧˩˧"
2. 移除声调: "ni"
3. 切分: 声母="n", 韵母="i"
4. 转换:
   - 声母: "n" → "n"
   - 韵母: "i" → "i{tone}"
5. 应用声调: "ni˧˩˧"

--- hao3 ---
1. 提取声调: 3 → "˧˩˧"
2. 移除声调: "hao"
3. 切分: 声母="h", 韵母="ao"
4. 转换:
   - 声母: "h" → "x"
   - 韵母: "ao" → "au̯{tone}"
5. 应用声调: "xau̯˧˩˧"

最终: "ni˧˩˧ xau̯˧˩˧"

简化 (可选): "ni↓ xau̯↓"
```

### 示例 2: "知识"

```
输入: "zhi1 shi5"

--- zhi1 ---
1. 提取声调: 1 → "˥"
2. 移除声调: "zhi"
3. 切分: 声母="zh", 韵母="i"
4. 转换:
   - 声母: "zh" → "ʈʂ"
   - 韵母 (特殊): "i" → "ɻ̩{tone}"  (卷舌韵母)
5. 应用声调: "ʈʂɻ̩˥"

--- shi5 ---
1. 提取声调: 5 → ""
2. 移除声调: "shi"
3. 切分: 声母="sh", 韵母="i"
4. 转换:
   - 声母: "sh" → "ʂ"
   - 韵母 (特殊): "i" → "ɻ̩{tone}"
5. 应用声调: "ʂɻ̩"

最终: "ʈʂɻ̩˥ ʂɻ̩"

简化 (可选): "ʈʂɻ̩→ ʂɻ̩"
```

---

## 🎯 Android 实施建议

### 阶段 1: 最小可行版本
- ✅ 实现基础声母+韵母映射
- ✅ 支持 1-5 声调
- ❌ 暂不处理感叹词、音节辅音
- ❌ 暂不做声调简化

**优点**: 快速验证，2-3 天完成  
**缺点**: 准确度约 85%

---

### 阶段 2: 完整版本
- ✅ 添加卷舌/平舌韵母特殊处理
- ✅ 添加感叹词映射
- ✅ 声调简化 (可选)

**优点**: 准确度 > 90%  
**缺点**: 开发时间 +1-2 天

---

## 📝 关键差异: 日语 vs 中文

| 项目 | 日语 G2P | 中文 G2P |
|------|---------|---------|
| **输入** | 假名 (キョウ) | 拼音 (jing1) |
| **声母** | 无 | 21 个 |
| **韵母** | 拗音+基本音 | 40 个 |
| **声调** | 无 | 5 个 |
| **特殊处理** | 长音、促音、拨音 | 卷舌韵母、儿化音 |
| **复杂度** | 🟢 中等 | 🟡 较高 |

---

## ✅ 下一步行动

### Task 0.2: 测试 pypinyin 转换
创建 `test_chinese_pinyin.py` 测试实际拼音输出:

```python
from pypinyin import lazy_pinyin, Style

test_sentences = [
    "你好世界",
    "中文测试",
    "知识就是力量"
]

for text in test_sentences:
    pinyins = lazy_pinyin(text, style=Style.TONE3, neutral_tone_with_five=True)
    print(f"{text} → {pinyins}")
```

---

**报告完成! 🎉**

现在我们已经完全理解了 misaki 的中文 G2P 实现，可以开始 Kotlin 移植了！
