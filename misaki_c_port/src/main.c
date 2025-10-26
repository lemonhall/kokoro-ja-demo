/**
 * main.c
 * 
 * Misaki C Port - 命令行工具
 * 多语言 G2P 转换器（文本 → 音素）
 * 
 * License: MIT
 */

#include "misaki.h"
#include "misaki_dict.h"
#include "misaki_tokenizer.h"
#include "misaki_g2p.h"
#include "misaki_trie.h"
#include "misaki_hmm.h"  // 添加：HMM 支持
#include "misaki_lang_detect.h"  // 添加：语言检测模块
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "0.3.0"
#define MAX_INPUT_LENGTH 4096

/* ============================================================================
 * 全局上下文
 * ========================================================================== */

typedef struct {
    // 词典
    EnDict *en_dict_us;
    ZhDict *zh_dict;
    ZhPhraseDict *zh_phrase_dict;  // 中文词组拼音词典（解决多音字）
    HmmModel *zh_hmm_model;        // 中文 HMM 模型（未登录词识别）
    
    // 分词器
    void *zh_tokenizer;
    void *ja_tokenizer;
    
    // 词汇 Trie 树
    Trie *zh_trie;
    Trie *ja_trie;
    
    // 语言检测器
    LangDetector *lang_detector;
} MisakiApp;

/* ============================================================================
 * 初始化和清理
 * ========================================================================== */

bool init_app(MisakiApp *app, const char *data_dir) {
    memset(app, 0, sizeof(MisakiApp));
    
    printf("🚀 初始化 Misaki G2P 引擎...\n\n");
    
    // 1. 加载英文词典
    char en_dict_path[512];
    snprintf(en_dict_path, sizeof(en_dict_path), "%s/en/us_dict.txt", data_dir);
    printf("📖 加载英文词典: %s\n", en_dict_path);
    
    app->en_dict_us = misaki_en_dict_load(en_dict_path);
    if (app->en_dict_us) {
        printf("   ✅ 成功加载 %d 个英文单词\n", app->en_dict_us->count);
    } else {
        printf("   ⚠️  无法加载英文词典（文件不存在或格式错误）\n");
    }
    
    // 2. 加载中文拼音词典
    char zh_dict_path[512];
    snprintf(zh_dict_path, sizeof(zh_dict_path), "%s/zh/pinyin_dict.txt", data_dir);
    printf("📖 加载中文拼音词典: %s\n", zh_dict_path);
    
    app->zh_dict = misaki_zh_dict_load(zh_dict_path);
    if (app->zh_dict) {
        printf("   ✅ 成功加载 %d 个汉字拼音\n", app->zh_dict->count);
    } else {
        printf("   ⚠️  无法加载中文拼音词典\n");
    }
    
    // ⭐ 2.5. 加载中文词组拼音词典（解决多音字）
    char zh_phrase_dict_path[512];
    snprintf(zh_phrase_dict_path, sizeof(zh_phrase_dict_path), "%s/zh/phrase_pinyin.txt", data_dir);
    printf("📖 加载中文词组拼音词典: %s\n", zh_phrase_dict_path);
    
    app->zh_phrase_dict = misaki_zh_phrase_dict_load(zh_phrase_dict_path);
    if (app->zh_phrase_dict) {
        int phrase_count = misaki_zh_phrase_dict_count(app->zh_phrase_dict);
        printf("   ✅ 成功加载 %d 个词组拼音 [解决多音字]\n", phrase_count);
    } else {
        printf("   ⚠️  无法加载词组拼音词典（将使用默认单字拼音）\n");
    }
    
    // ⭐ 2.6. 加载中文 HMM 模型（未登录词识别）
    char zh_hmm_path[512];
    snprintf(zh_hmm_path, sizeof(zh_hmm_path), "%s/zh/hmm_prob_emit.txt", data_dir);
    printf("📖 加载中文 HMM 模型: %s\n", zh_hmm_path);
    
    app->zh_hmm_model = misaki_hmm_load(zh_hmm_path);
    if (app->zh_hmm_model) {
        printf("   ✅ 成功加载 HMM 模型 (%d 个字符) [未登录词识别]\n", 
               app->zh_hmm_model->total_chars);
    } else {
        printf("   ⚠️  无法加载 HMM 模型（未登录词将无法处理）\n");
    }
    
    // 3. 加载中文词汇词典（用于分词）
    // 优先级：dict_merged.txt > dict_full.txt > dict.txt
    if (app->zh_dict) {
        char zh_word_dict_path[512];
        char zh_word_dict_full_path[512];
        char zh_word_dict_merged_path[512];
        
        // 尝试加载合并词典（最优先）
        snprintf(zh_word_dict_merged_path, sizeof(zh_word_dict_merged_path), 
                 "%s/zh/dict_merged.txt", data_dir);
        snprintf(zh_word_dict_full_path, sizeof(zh_word_dict_full_path), 
                 "%s/zh/dict_full.txt", data_dir);
        snprintf(zh_word_dict_path, sizeof(zh_word_dict_path), 
                 "%s/zh/dict.txt", data_dir);
        
        // 检查词典文件存在性
        FILE *test_file = fopen(zh_word_dict_merged_path, "r");
        bool use_merged_dict = (test_file != NULL);
        if (test_file) {
            fclose(test_file);
        }
        
        const char *selected_dict = NULL;
        const char *dict_type = NULL;
        
        if (use_merged_dict) {
            selected_dict = zh_word_dict_merged_path;
            dict_type = "合并词典（含专有名词）";
        } else {
            test_file = fopen(zh_word_dict_full_path, "r");
            bool use_full_dict = (test_file != NULL);
            if (test_file) {
                fclose(test_file);
            }
            
            if (use_full_dict) {
                selected_dict = zh_word_dict_full_path;
                dict_type = "大词典";
            } else {
                selected_dict = zh_word_dict_path;
                dict_type = "基础词典";
            }
        }
        
        printf("📖 加载中文词汇词典 (%s): %s\n", dict_type, selected_dict);
        
        app->zh_trie = misaki_trie_create();
        int word_count = misaki_trie_load_from_file(app->zh_trie, selected_dict, "word freq");
        if (word_count > 0) {
            printf("   ✅ 成功加载 %d 个中文词汇 [%s]\n", word_count, dict_type);
            
            // 创建中文分词器
            ZhTokenizerConfig config = {
                .dict_trie = app->zh_trie,
                .enable_hmm = true,  // 启用 HMM
                .hmm_model = app->zh_hmm_model,  // 传入 HMM 模型
                .enable_userdict = false,
                .user_trie = NULL
            };
            app->zh_tokenizer = misaki_zh_tokenizer_create(&config);
            if (app->zh_tokenizer) {
                printf("   ✅ 中文分词器初始化成功\n");
            }
        } else {
            printf("   ⚠️  无法加载中文词汇词典\n");
        }
    }
    
    // 4. 加载日文词汇+读音词典
    char ja_dict_path[512];
    snprintf(ja_dict_path, sizeof(ja_dict_path), "%s/ja/ja_pron_dict.tsv", data_dir);
    printf("📖 加载日文词汇+读音词典: %s\n", ja_dict_path);
    
    app->ja_trie = misaki_trie_create();
    int ja_word_count = misaki_trie_load_ja_pron_dict(app->ja_trie, ja_dict_path);
    if (ja_word_count > 0) {
        printf("   ✅ 成功加载 %d 个日文词汇（含读音）\n", ja_word_count);
        
        // 创建日文分词器
        JaTokenizerConfig ja_config = {
            .dict_trie = app->ja_trie,
            .use_simple_model = true,
            .unidic_path = NULL
        };
        app->ja_tokenizer = misaki_ja_tokenizer_create(&ja_config);
        if (app->ja_tokenizer) {
            printf("   ✅ 日文分词器初始化成功（带读音标注）\n");
        }
    } else {
        printf("   ⚠️  无法加载日文词典（使用简化版分词）\n");
        // 降级到简化版
        misaki_trie_insert(app->ja_trie, "こんにちは", 1.0, NULL);
        misaki_trie_insert(app->ja_trie, "世界", 1.0, NULL);
        misaki_trie_insert(app->ja_trie, "です", 1.0, NULL);
        
        JaTokenizerConfig ja_config = {
            .dict_trie = app->ja_trie,
            .use_simple_model = true,
            .unidic_path = NULL
        };
        app->ja_tokenizer = misaki_ja_tokenizer_create(&ja_config);
        if (app->ja_tokenizer) {
            printf("   ✅ 日文分词器初始化成功（简化版）\n");
        }
    }
    
    // 5. 初始化语言检测器
    printf("🔍 初始化语言检测器...\n");
    LangDetectorConfig detector_config = {
        .enable_ngram = true,
        .enable_tokenization = false,  // 可选：启用分词质量检测
        .confidence_threshold = 0.5f,
        .zh_tokenizer = app->zh_tokenizer,
        .ja_tokenizer = app->ja_tokenizer
    };
    app->lang_detector = misaki_lang_detector_create(&detector_config);
    if (app->lang_detector) {
        printf("   ✅ 语言检测器初始化成功\n");
    } else {
        printf("   ⚠️  语言检测器初始化失败（将使用快速检测）\n");
    }
    
    printf("\n");
    return true;
}

void cleanup_app(MisakiApp *app) {
    if (app->en_dict_us) {
        misaki_en_dict_free(app->en_dict_us);
    }
    if (app->zh_dict) {
        misaki_zh_dict_free(app->zh_dict);
    }
    if (app->zh_phrase_dict) {
        misaki_zh_phrase_dict_free(app->zh_phrase_dict);
    }
    if (app->zh_hmm_model) {
        misaki_hmm_free(app->zh_hmm_model);
    }
    if (app->zh_tokenizer) {
        misaki_zh_tokenizer_free(app->zh_tokenizer);
    }
    if (app->ja_tokenizer) {
        misaki_ja_tokenizer_free(app->ja_tokenizer);
    }
    if (app->zh_trie) {
        misaki_trie_free(app->zh_trie);
    }
    if (app->ja_trie) {
        misaki_trie_free(app->ja_trie);
    }
    if (app->lang_detector) {
        misaki_lang_detector_free(app->lang_detector);
    }
}

/* ============================================================================
 * 语言检测封装（使用新模块）
 * ========================================================================== */

/**
 * 将新的语言类型转换为旧的枚举（兼容性）
 */
static MisakiLanguage convert_lang_type(MisakiLanguage new_lang) {
    // 新模块的枚举值与旧版本兼容，直接返回
    return new_lang;
}

/* ============================================================================
 * G2P 处理
 * ========================================================================== */

void process_text(MisakiApp *app, const char *text) {
    process_text_ex(app, text, false);
}

void process_text_quiet(MisakiApp *app, const char *text) {
    process_text_ex(app, text, true);
}

void process_text_ex(MisakiApp *app, const char *text, bool quiet) {
    if (!text || !*text) {
        return;
    }
    
    if (!quiet) {
        printf("════════════════════════════════════════════════════════════\n");
        printf("📝 输入文本: %s\n", text);
        printf("════════════════════════════════════════════════════════════\n\n");
    }
    
    // 检测语言（使用新模块）
    MisakiLanguage lang;
    float confidence = 0.0f;
    const char *reason = "未检测";
    
    if (app->lang_detector) {
        // 使用完整检测器
        LangDetectResult result = misaki_lang_detect_full(app->lang_detector, text);
        lang = result.language;
        confidence = result.confidence;
        reason = result.reason;
        
        if (!quiet) {
            printf("🌏 检测语言: %s (置信度: %.2f%%, 原因: %s)\n", 
                   misaki_language_name(lang), confidence * 100, reason);
            
            // 显示字符集统计
            if (result.charset.total_chars > 0) {
                printf("📊 字符统计: ");
                if (result.charset.hiragana_count > 0) {
                    printf("平假名=%d ", result.charset.hiragana_count);
                }
                if (result.charset.katakana_count > 0) {
                    printf("片假名=%d ", result.charset.katakana_count);
                }
                if (result.charset.kanji_count > 0) {
                    printf("汉字=%d ", result.charset.kanji_count);
                }
                if (result.charset.latin_count > 0) {
                    printf("拉丁=%d ", result.charset.latin_count);
                }
                printf("总计=%d\n", result.charset.total_chars);
            }
        }
    } else {
        // 降级到快速检测
        lang = misaki_lang_detect_quick(text);
        if (!quiet) {
            printf("🌏 检测语言: %s (快速模式)\n", misaki_language_name(lang));
        }
    }
    if (!quiet) {
        printf("\n");
    }
    
    // 根据语言调用不同的 G2P
    MisakiTokenList *tokens = NULL;
    G2POptions options = misaki_g2p_default_options();
    
    switch (lang) {
        case LANG_ENGLISH:
            if (app->en_dict_us) {
                if (!quiet) {
                    printf("🔤 英文 G2P 转换中...\n\n");
                }
                tokens = misaki_en_g2p(app->en_dict_us, text, &options);
            } else {
                if (!quiet) {
                    printf("❌ 英文词典未加载\n");
                }
            }
            break;
            
        case LANG_CHINESE:
            if (app->zh_dict && app->zh_tokenizer) {
                if (!quiet) {
                    printf("🔤 中文 G2P 转换中...\n\n");
                }
                tokens = misaki_zh_g2p(app->zh_dict, app->zh_phrase_dict, app->zh_tokenizer, text, &options);
            } else {
                if (!quiet) {
                    printf("❌ 中文词典或分词器未加载\n");
                }
            }
            break;
            
        case LANG_JAPANESE:
            if (app->ja_tokenizer && app->ja_trie) {
                if (!quiet) {
                    printf("🔤 日文 G2P 转换中...\n\n");
                }
                // ⭐ 使用统一的 G2P 函数
                tokens = misaki_ja_g2p(app->ja_trie, app->ja_tokenizer, text, &options);
            } else {
                if (!quiet) {
                    printf("❌ 日文分词器或词典未加载\n");
                }
            }
            break;
            
        default:
            if (!quiet) {
                printf("❌ 无法识别语言\n");
            }
            break;
    }
    
    // 显示结果
    if (tokens) {
        if (quiet) {
            // 安静模式：仅输出音素序列
            char *merged = misaki_merge_phonemes(tokens, " ");
            if (merged) {
                printf("%s\n", merged);
                free(merged);
            }
        } else {
            // 正常模式：完整输出
            printf("📊 分词结果:\n");
            printf("────────────────────────────────────────────────────────────\n");
            misaki_g2p_print(tokens, true);
            printf("────────────────────────────────────────────────────────────\n\n");
            
            // 合并音素
            char *merged = misaki_merge_phonemes(tokens, " ");
            if (merged) {
                printf("🎵 音素序列: %s\n\n", merged);
                free(merged);
            }
            
            // 统计信息
            int total_phonemes = 0;
            int oov_count = 0;
            double avg_phonemes = 0.0;
            misaki_g2p_stats(tokens, &total_phonemes, &oov_count, &avg_phonemes);
            
            printf("📈 统计信息:\n");
            printf("   - 总词数: %d\n", tokens->count);
            printf("   - 总音素数: %d\n", total_phonemes);
            printf("   - 未登录词: %d\n", oov_count);
            printf("   - 平均音素/词: %.2f\n", avg_phonemes);
        }
        
        misaki_token_list_free(tokens);
    }
    
    if (!quiet) {
        printf("\n");
    }
}

/* ============================================================================
 * 交互模式
 * ========================================================================== */

void interactive_mode(MisakiApp *app) {
    char input[MAX_INPUT_LENGTH];
    
    printf("════════════════════════════════════════════════════════════\n");
    printf("  Misaki G2P - 交互模式\n");
    printf("  版本: %s\n", VERSION);
    printf("════════════════════════════════════════════════════════════\n\n");
    printf("💡 使用说明:\n");
    printf("   - 输入文本，按回车查看 G2P 转换结果\n");
    printf("   - 支持中文、英文、日文（带读音标注）\n");
    printf("   - 输入 'quit' 或 'exit' 退出\n");
    printf("   - 输入 'help' 查看帮助\n");
    printf("   - 输入 'test' 查看测试样例\n\n");
    
    while (1) {
        printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
        printf("请输入文本> ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        // 移除换行符
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }
        
        // 检查退出命令
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            printf("\n👋 再见！\n");
            break;
        }
        
        // 检查帮助命令
        if (strcmp(input, "help") == 0) {
            printf("\n📚 帮助信息:\n");
            printf("   支持的语言:\n");
            printf("     - 英文: Hello world\n");
            printf("     - 中文: 你好世界\n");
            printf("     - 日文: こんにちは世界 / 私は学生です\n\n");
            printf("   示例:\n");
            printf("     输入> Hello world\n");
            printf("     输出> həlˈO wˈɜɹld\n\n");
            printf("     输入> 私は学生です\n");
            printf("     输出> βatakɯɕi βa ɡakɯseː desɨ\n\n");
            continue;
        }
        
        // 检查测试命令
        if (strcmp(input, "test") == 0) {
            printf("\n🧪 测试样例:\n\n");
            
            const char *test_cases[] = {
                "Hello world",
                "你好世界",
                "私は学生です",
                "コーヒーを飲みます",
                "ありがとうございます"
            };
            
            for (int i = 0; i < 5; i++) {
                printf("测试 %d: %s\n", i+1, test_cases[i]);
                process_text(app, test_cases[i]);
            }
            continue;
        }
        
        // 空输入
        if (strlen(input) == 0) {
            continue;
        }
        
        // 处理文本
        printf("\n");
        process_text(app, input);
    }
}

/* ============================================================================
 * 命令行模式
 * ========================================================================== */

void print_usage(const char *prog_name) {
    printf("用法: %s [选项] [文本]\n\n", prog_name);
    printf("选项:\n");
    printf("  -h, --help           显示帮助信息\n");
    printf("  -v, --version        显示版本信息\n");
    printf("  -d, --data <目录>    指定数据目录（默认: ../extracted_data）\n");
    printf("  -i, --interactive    交互模式\n");
    printf("  -q, --quiet          安静模式（仅输出音素）\n\n");
    printf("示例:\n");
    printf("  %s \"Hello world\"              # 转换英文文本\n", prog_name);
    printf("  %s \"你好世界\"                  # 转换中文文本\n", prog_name);
    printf("  %s -i                          # 进入交互模式\n", prog_name);
    printf("  %s -d ./data \"Hello\"           # 指定数据目录\n", prog_name);
    printf("  %s -q \"こんにちは\"              # 安静模式，仅输出音素\n\n", prog_name);
}

/* ============================================================================
 * 主函数
 * ========================================================================== */

int main(int argc, char *argv[]) {
    MisakiApp app;
    const char *data_dir = "../extracted_data";
    bool interactive = false;
    bool quiet_mode = false;  // 安静模式
    const char *text_to_process = NULL;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("Misaki G2P v%s\n", VERSION);
            return 0;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--data") == 0) {
            if (i + 1 < argc) {
                data_dir = argv[++i];
            } else {
                fprintf(stderr, "错误: -d 选项需要指定目录路径\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
            interactive = true;
        } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            quiet_mode = true;
        } else {
            text_to_process = argv[i];
        }
    }
    
    // 初始化应用
    if (!init_app(&app, data_dir)) {
        if (!quiet_mode) {
            fprintf(stderr, "错误: 初始化失败\n");
        }
        return 1;
    }
    
    // 执行模式
    if (interactive || text_to_process == NULL) {
        interactive_mode(&app);
    } else {
        if (quiet_mode) {
            process_text_quiet(&app, text_to_process);
        } else {
            process_text(&app, text_to_process);
        }
    }
    
    // 清理
    cleanup_app(&app);
    
    return 0;
}
