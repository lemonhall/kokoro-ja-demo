/**
 * misaki_api.c
 * 
 * Misaki G2P - 导出 API 实现
 * 
 * License: MIT
 */

#include "misaki_api.h"
#include "misaki_g2p.h"
#include "misaki_dict.h"
#include "misaki_tokenizer.h"
#include "misaki_trie.h"
#include "misaki_hmm.h"
#include "misaki_lang_detect.h"
#include "misaki_g2p_qya.h"      // 昆雅语 G2P
#include "misaki_tokenizer_qya.h" // 昆雅语分词器
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define VERSION "0.3.0"

// 全局状态（单例）
static struct {
    bool initialized;
    EnDict *en_dict;
    ZhDict *zh_dict;
    ZhPhraseDict *zh_phrase_dict;
    HmmModel *zh_hmm_model;
    void *zh_tokenizer;
    void *ja_tokenizer;
    Trie *zh_trie;
    Trie *ja_trie;
    LangDetector *lang_detector;
} g_misaki = {0};

/**
 * 初始化 Misaki G2P 引擎
 */
MISAKI_API int misaki_init(const char *data_dir) {
    if (g_misaki.initialized) {
        return 0;  // 已初始化
    }
    
    if (!data_dir) {
        data_dir = "../extracted_data";
    }
    
    // 1. 加载英文词典
    char path[512];
    snprintf(path, sizeof(path), "%s/en/us_dict.txt", data_dir);
    g_misaki.en_dict = misaki_en_dict_load(path);
    
    // 2. 加载中文词典
    snprintf(path, sizeof(path), "%s/zh/pinyin_dict.txt", data_dir);
    g_misaki.zh_dict = misaki_zh_dict_load(path);
    
    snprintf(path, sizeof(path), "%s/zh/phrase_pinyin.txt", data_dir);
    g_misaki.zh_phrase_dict = misaki_zh_phrase_dict_load(path);
    
    snprintf(path, sizeof(path), "%s/zh/hmm_prob_emit.txt", data_dir);
    g_misaki.zh_hmm_model = misaki_hmm_load(path);
    
    // 加载中文词汇
    snprintf(path, sizeof(path), "%s/zh/dict_merged.txt", data_dir);
    g_misaki.zh_trie = misaki_trie_create();
    misaki_trie_load_from_file(g_misaki.zh_trie, path, "word freq");
    
    // 创建中文分词器
    if (g_misaki.zh_dict && g_misaki.zh_trie) {
        ZhTokenizerConfig config = {
            .dict_trie = g_misaki.zh_trie,
            .enable_hmm = true,
            .hmm_model = g_misaki.zh_hmm_model,
            .enable_userdict = false,
            .user_trie = NULL
        };
        g_misaki.zh_tokenizer = misaki_zh_tokenizer_create(&config);
    }
    
    // 3. 加载日文词典
    snprintf(path, sizeof(path), "%s/ja/ja_pron_dict.tsv", data_dir);
    g_misaki.ja_trie = misaki_trie_create();
    int ja_count = misaki_trie_load_ja_pron_dict(g_misaki.ja_trie, path);
    
    if (ja_count > 0) {
        JaTokenizerConfig ja_config = {
            .dict_trie = g_misaki.ja_trie,
            .use_simple_model = true,
            .unidic_path = NULL
        };
        g_misaki.ja_tokenizer = misaki_ja_tokenizer_create(&ja_config);
    }
    
    // 4. 初始化语言检测器
    LangDetectorConfig detector_config = {
        .enable_ngram = true,
        .enable_tokenization = false,
        .confidence_threshold = 0.5f,
        .zh_tokenizer = g_misaki.zh_tokenizer,
        .ja_tokenizer = g_misaki.ja_tokenizer
    };
    g_misaki.lang_detector = misaki_lang_detector_create(&detector_config);
    
    // 5. 初始化昆雅语 G2P（无需词典）
    misaki_g2p_qya_init();
    misaki_tokenizer_qya_init();
    
    g_misaki.initialized = true;
    return 0;
}

/**
 * 文本转音素（自动检测语言）
 */
MISAKI_API int misaki_text_to_phonemes(
    const char *text,
    char *output_buffer,
    int buffer_size
) {
    if (!g_misaki.initialized) {
        return -1;
    }
    
    if (!text || !output_buffer || buffer_size <= 0) {
        return -1;
    }
    
    // 检测语言
    MisakiLanguage lang = LANG_UNKNOWN;
    if (g_misaki.lang_detector) {
        LangDetectResult result = misaki_lang_detect_full(g_misaki.lang_detector, text);
        lang = result.language;
        fprintf(stderr, "[DEBUG] 检测语言: %s (置信度: %.2f%%)\n", 
                misaki_language_name(lang), result.confidence * 100);
    } else {
        lang = misaki_lang_detect_quick(text);
        fprintf(stderr, "[DEBUG] 快速检测语言: %s\n", misaki_language_name(lang));
    }
    
    // 根据语言调用 G2P
    MisakiTokenList *tokens = NULL;
    G2POptions options = misaki_g2p_default_options();
    
    switch (lang) {
        case LANG_ENGLISH:
            if (g_misaki.en_dict) {
                tokens = misaki_en_g2p(g_misaki.en_dict, text, &options);
            }
            break;
            
        case LANG_CHINESE:
            if (g_misaki.zh_dict && g_misaki.zh_tokenizer) {
                tokens = misaki_zh_g2p(g_misaki.zh_dict, g_misaki.zh_phrase_dict, 
                                      g_misaki.zh_tokenizer, text, &options);
            }
            break;
            
        case LANG_JAPANESE:
            if (g_misaki.ja_tokenizer && g_misaki.ja_trie) {
                tokens = misaki_ja_g2p(g_misaki.ja_trie, g_misaki.ja_tokenizer, text, &options);
            }
            break;
            
        default:
            return -1;
    }
    
    if (!tokens) {
        return -1;
    }
    
    // 合并音素
    char *merged = misaki_merge_phonemes(tokens, " ");
    if (merged) {
        strncpy(output_buffer, merged, buffer_size - 1);
        output_buffer[buffer_size - 1] = '\0';
        free(merged);
        misaki_token_list_free(tokens);
        return 0;
    }
    
    misaki_token_list_free(tokens);
    return -1;
}

/**
 * 文本转音素（指定语言）
 */
MISAKI_API int misaki_text_to_phonemes_lang(
    const char *text,
    const char *lang,
    char *output_buffer,
    int buffer_size
) {
    if (!g_misaki.initialized) {
        return -1;
    }
    
    if (!text || !lang || !output_buffer || buffer_size <= 0) {
        return -1;
    }
    
    // 解析语言代码
    MisakiLanguage detected_lang = LANG_UNKNOWN;
    if (strcmp(lang, "ja") == 0 || strcmp(lang, "jp") == 0) {
        detected_lang = LANG_JAPANESE;
    } else if (strcmp(lang, "zh") == 0 || strcmp(lang, "cn") == 0) {
        detected_lang = LANG_CHINESE;
    } else if (strcmp(lang, "en") == 0) {
        detected_lang = LANG_ENGLISH;
    } else if (strcmp(lang, "qya") == 0 || strcmp(lang, "quenya") == 0) {
        detected_lang = LANG_QUENYA;
    } else {
        return -1;
    }
    
    // 调用 G2P
    MisakiTokenList *tokens = NULL;
    G2POptions options = misaki_g2p_default_options();
    
    switch (detected_lang) {
        case LANG_ENGLISH:
            if (g_misaki.en_dict) {
                tokens = misaki_en_g2p(g_misaki.en_dict, text, &options);
            }
            break;
            
        case LANG_CHINESE:
            if (g_misaki.zh_dict && g_misaki.zh_tokenizer) {
                tokens = misaki_zh_g2p(g_misaki.zh_dict, g_misaki.zh_phrase_dict,
                                      g_misaki.zh_tokenizer, text, &options);
            }
            break;
            
        case LANG_JAPANESE:
            if (g_misaki.ja_tokenizer && g_misaki.ja_trie) {
                tokens = misaki_ja_g2p(g_misaki.ja_trie, g_misaki.ja_tokenizer, text, &options);
            }
            break;
            
        case LANG_QUENYA:
            {
                // 昆雅语特殊处理：直接调用 G2P，不需要词典
                char* phonemes_str = NULL;
                if (misaki_g2p_qya_convert(text, &phonemes_str) == 0 && phonemes_str) {
                    strncpy(output_buffer, phonemes_str, buffer_size - 1);
                    output_buffer[buffer_size - 1] = '\0';
                    free(phonemes_str);
                    return 0;
                }
                return -1;
            }
            break;
            
        default:
            return -1;
    }
    
    if (!tokens) {
        return -1;
    }
    
    // 合并音素
    char *merged = misaki_merge_phonemes(tokens, " ");
    if (merged) {
        strncpy(output_buffer, merged, buffer_size - 1);
        output_buffer[buffer_size - 1] = '\0';
        free(merged);
        misaki_token_list_free(tokens);
        return 0;
    }
    
    misaki_token_list_free(tokens);
    return -1;
}

/**
 * 清理
 */
MISAKI_API void misaki_cleanup(void) {
    if (!g_misaki.initialized) {
        return;
    }
    
    if (g_misaki.en_dict) {
        misaki_en_dict_free(g_misaki.en_dict);
    }
    if (g_misaki.zh_dict) {
        misaki_zh_dict_free(g_misaki.zh_dict);
    }
    if (g_misaki.zh_phrase_dict) {
        misaki_zh_phrase_dict_free(g_misaki.zh_phrase_dict);
    }
    if (g_misaki.zh_hmm_model) {
        misaki_hmm_free(g_misaki.zh_hmm_model);
    }
    if (g_misaki.zh_tokenizer) {
        misaki_zh_tokenizer_free(g_misaki.zh_tokenizer);
    }
    if (g_misaki.ja_tokenizer) {
        misaki_ja_tokenizer_free(g_misaki.ja_tokenizer);
    }
    if (g_misaki.zh_trie) {
        misaki_trie_free(g_misaki.zh_trie);
    }
    if (g_misaki.ja_trie) {
        misaki_trie_free(g_misaki.ja_trie);
    }
    if (g_misaki.lang_detector) {
        misaki_lang_detector_free(g_misaki.lang_detector);
    }
    
    // 清理昆雅语 G2P
    misaki_g2p_qya_cleanup();
    misaki_tokenizer_qya_cleanup();
    
    memset(&g_misaki, 0, sizeof(g_misaki));
}

/**
 * 获取版本号
 */
MISAKI_API const char* misaki_get_version(void) {
    return VERSION;
}
