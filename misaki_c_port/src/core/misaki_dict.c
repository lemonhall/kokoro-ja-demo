/**
 * misaki_dict.c
 * 
 * Misaki C Port - Dictionary Loading and Query
 * 词典加载与查询实现
 * 
 * License: MIT
 */

#include "misaki_dict.h"
#include "misaki_string.h"
#include "misaki_trie.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * 英文词典实现
 * ========================================================================== */

EnDict* misaki_en_dict_load(const char *file_path) {
    if (!file_path) {
        return NULL;
    }
    
    // 创建词典对象
    EnDict *dict = (EnDict *)malloc(sizeof(EnDict));
    if (!dict) {
        return NULL;
    }
    
    dict->count = 0;
    dict->capacity = 1000;  // 初始容量
    dict->entries = (EnDictEntry *)malloc(sizeof(EnDictEntry) * dict->capacity);
    if (!dict->entries) {
        free(dict);
        return NULL;
    }
    
    // 打开 TSV 解析器
    TSVParser *parser = misaki_tsv_parser_create(file_path);
    if (!parser) {
        free(dict->entries);
        free(dict);
        return NULL;
    }
    
    // 逐行读取
    MisakiStringView fields[10];
    while (true) {
        int field_count = misaki_tsv_parser_next_line(parser, fields, 10);
        
        if (field_count == 0) {
            break;  // 文件结束
        }
        
        if (field_count < 2) {
            continue;  // 跳过格式错误的行
        }
        
        // 扩容（如果需要）
        if (dict->count >= dict->capacity) {
            int new_capacity = dict->capacity * 2;
            EnDictEntry *new_entries = (EnDictEntry *)realloc(
                dict->entries, sizeof(EnDictEntry) * new_capacity);
            if (!new_entries) {
                break;  // 内存不足，停止加载
            }
            dict->entries = new_entries;
            dict->capacity = new_capacity;
        }
        
        // 复制单词和音素
        EnDictEntry *entry = &dict->entries[dict->count];
        entry->word = strndup(fields[0].data, fields[0].length);
        entry->phonemes = strndup(fields[1].data, fields[1].length);
        
        if (!entry->word || !entry->phonemes) {
            free(entry->word);
            free(entry->phonemes);
            break;
        }
        
        // 转小写（英文词典查询不区分大小写）
        misaki_strlower(entry->word);
        
        dict->count++;
    }
    
    misaki_tsv_parser_free(parser);
    return dict;
}

void misaki_en_dict_free(EnDict *dict) {
    if (!dict) {
        return;
    }
    
    for (int i = 0; i < dict->count; i++) {
        free(dict->entries[i].word);
        free(dict->entries[i].phonemes);
    }
    
    free(dict->entries);
    free(dict);
}

const char* misaki_en_dict_lookup(const EnDict *dict, const char *word) {
    if (!dict || !word) {
        return NULL;
    }
    
    // 转小写进行查找
    char *lower_word = misaki_strdup(word);
    if (!lower_word) {
        return NULL;
    }
    misaki_strlower(lower_word);
    
    // 线性查找（后续可优化为二分查找或哈希表）
    const char *result = NULL;
    for (int i = 0; i < dict->count; i++) {
        if (strcmp(dict->entries[i].word, lower_word) == 0) {
            result = dict->entries[i].phonemes;
            break;
        }
    }
    
    free(lower_word);
    return result;
}

int misaki_en_dict_lookup_batch(const EnDict *dict, 
                                 const char **words, 
                                 int count, 
                                 const char **results) {
    if (!dict || !words || !results || count <= 0) {
        return 0;
    }
    
    int found_count = 0;
    for (int i = 0; i < count; i++) {
        results[i] = misaki_en_dict_lookup(dict, words[i]);
        if (results[i]) {
            found_count++;
        }
    }
    
    return found_count;
}

void misaki_en_dict_stats(const EnDict *dict, 
                          int *total_entries,
                          double *avg_word_length,
                          double *avg_phoneme_length) {
    if (!dict) {
        return;
    }
    
    if (total_entries) {
        *total_entries = dict->count;
    }
    
    if (avg_word_length || avg_phoneme_length) {
        double total_word_len = 0;
        double total_phoneme_len = 0;
        
        for (int i = 0; i < dict->count; i++) {
            total_word_len += strlen(dict->entries[i].word);
            total_phoneme_len += strlen(dict->entries[i].phonemes);
        }
        
        if (avg_word_length) {
            *avg_word_length = dict->count > 0 ? total_word_len / dict->count : 0;
        }
        
        if (avg_phoneme_length) {
            *avg_phoneme_length = dict->count > 0 ? total_phoneme_len / dict->count : 0;
        }
    }
}

/* ============================================================================
 * 中文词典实现
 * ========================================================================== */

ZhDict* misaki_zh_dict_load(const char *file_path) {
    if (!file_path) {
        return NULL;
    }
    
    ZhDict *dict = (ZhDict *)malloc(sizeof(ZhDict));
    if (!dict) {
        return NULL;
    }
    
    dict->count = 0;
    dict->capacity = 5000;  // 初始容量（汉字数量较多）
    dict->entries = (ZhDictEntry *)malloc(sizeof(ZhDictEntry) * dict->capacity);
    if (!dict->entries) {
        free(dict);
        return NULL;
    }
    
    TSVParser *parser = misaki_tsv_parser_create(file_path);
    if (!parser) {
        free(dict->entries);
        free(dict);
        return NULL;
    }
    
    MisakiStringView fields[10];
    while (true) {
        int field_count = misaki_tsv_parser_next_line(parser, fields, 10);
        
        if (field_count == 0) {
            break;
        }
        
        if (field_count < 2) {
            continue;
        }
        
        // 扩容
        if (dict->count >= dict->capacity) {
            int new_capacity = dict->capacity * 2;
            ZhDictEntry *new_entries = (ZhDictEntry *)realloc(
                dict->entries, sizeof(ZhDictEntry) * new_capacity);
            if (!new_entries) {
                break;
            }
            dict->entries = new_entries;
            dict->capacity = new_capacity;
        }
        
        ZhDictEntry *entry = &dict->entries[dict->count];
        
        // 解析汉字（第一个字段应该是单个汉字）
        uint32_t hanzi;
        int bytes = misaki_utf8_decode(fields[0].data, &hanzi);
        if (bytes == 0) {
            continue;  // 无效的 UTF-8
        }
        entry->hanzi = hanzi;
        
        // 解析拼音（可能有多个，逗号分隔）
        char *pinyin_str = strndup(fields[1].data, fields[1].length);
        if (!pinyin_str) {
            break;
        }
        
        // 分割拼音（按逗号）
        entry->pinyin_count = 0;
        entry->pinyins = (char **)malloc(sizeof(char *) * 8);  // 最多 8 个读音
        if (!entry->pinyins) {
            free(pinyin_str);
            break;
        }
        
        char *token = strtok(pinyin_str, ",");
        while (token && entry->pinyin_count < 8) {
            entry->pinyins[entry->pinyin_count] = misaki_strdup(token);
            if (!entry->pinyins[entry->pinyin_count]) {
                break;
            }
            entry->pinyin_count++;
            token = strtok(NULL, ",");
        }
        
        free(pinyin_str);
        
        if (entry->pinyin_count > 0) {
            dict->count++;
        }
    }
    
    misaki_tsv_parser_free(parser);
    return dict;
}

void misaki_zh_dict_free(ZhDict *dict) {
    if (!dict) {
        return;
    }
    
    for (int i = 0; i < dict->count; i++) {
        for (int j = 0; j < dict->entries[i].pinyin_count; j++) {
            free(dict->entries[i].pinyins[j]);
        }
        free(dict->entries[i].pinyins);
    }
    
    free(dict->entries);
    free(dict);
}

bool misaki_zh_dict_lookup(const ZhDict *dict, 
                           uint32_t hanzi,
                           const char ***pinyins,
                           int *count) {
    if (!dict || !pinyins || !count) {
        return false;
    }
    
    // 线性查找
    for (int i = 0; i < dict->count; i++) {
        if (dict->entries[i].hanzi == hanzi) {
            *pinyins = (const char **)dict->entries[i].pinyins;
            *count = dict->entries[i].pinyin_count;
            return true;
        }
    }
    
    return false;
}

const char* misaki_zh_dict_lookup_first(const ZhDict *dict, uint32_t hanzi) {
    const char **pinyins;
    int count;
    
    if (misaki_zh_dict_lookup(dict, hanzi, &pinyins, &count) && count > 0) {
        return pinyins[0];
    }
    
    return NULL;
}

bool misaki_zh_is_hanzi(uint32_t codepoint) {
    // CJK 统一汉字基本区：U+4E00 - U+9FFF
    if (codepoint >= 0x4E00 && codepoint <= 0x9FFF) {
        return true;
    }
    
    // CJK 扩展 A：U+3400 - U+4DBF
    if (codepoint >= 0x3400 && codepoint <= 0x4DBF) {
        return true;
    }
    
    // CJK 扩展 B-F：U+20000 - U+2EBEF
    if (codepoint >= 0x20000 && codepoint <= 0x2EBEF) {
        return true;
    }
    
    return false;
}

void misaki_zh_dict_stats(const ZhDict *dict,
                          int *total_hanzi,
                          int *total_pinyins,
                          int *multi_pinyin_count) {
    if (!dict) {
        return;
    }
    
    if (total_hanzi) {
        *total_hanzi = dict->count;
    }
    
    if (total_pinyins || multi_pinyin_count) {
        int t_pinyins = 0;
        int multi_count = 0;
        
        for (int i = 0; i < dict->count; i++) {
            t_pinyins += dict->entries[i].pinyin_count;
            if (dict->entries[i].pinyin_count > 1) {
                multi_count++;
            }
        }
        
        if (total_pinyins) {
            *total_pinyins = t_pinyins;
        }
        
        if (multi_pinyin_count) {
            *multi_pinyin_count = multi_count;
        }
    }
}

/* ============================================================================
 * 日文词汇表实现
 * ========================================================================== */

JaVocab* misaki_ja_vocab_load(const char *file_path) {
    if (!file_path) {
        return NULL;
    }
    
    JaVocab *vocab = (JaVocab *)malloc(sizeof(JaVocab));
    if (!vocab) {
        return NULL;
    }
    
    vocab->count = 0;
    vocab->capacity = 1000;
    vocab->entries = (JaWordEntry *)malloc(sizeof(JaWordEntry) * vocab->capacity);
    if (!vocab->entries) {
        free(vocab);
        return NULL;
    }
    
    TSVParser *parser = misaki_tsv_parser_create(file_path);
    if (!parser) {
        free(vocab->entries);
        free(vocab);
        return NULL;
    }
    
    MisakiStringView fields[10];
    while (true) {
        int field_count = misaki_tsv_parser_next_line(parser, fields, 10);
        
        if (field_count == 0) {
            break;
        }
        
        if (field_count < 1 || fields[0].length == 0) {
            continue;
        }
        
        // 扩容
        if (vocab->count >= vocab->capacity) {
            int new_capacity = vocab->capacity * 2;
            JaWordEntry *new_entries = (JaWordEntry *)realloc(
                vocab->entries, sizeof(JaWordEntry) * new_capacity);
            if (!new_entries) {
                break;
            }
            vocab->entries = new_entries;
            vocab->capacity = new_capacity;
        }
        
        // 复制词汇
        vocab->entries[vocab->count].word = strndup(fields[0].data, fields[0].length);
        if (!vocab->entries[vocab->count].word) {
            break;
        }
        
        vocab->count++;
    }
    
    misaki_tsv_parser_free(parser);
    return vocab;
}

void misaki_ja_vocab_free(JaVocab *vocab) {
    if (!vocab) {
        return;
    }
    
    for (int i = 0; i < vocab->count; i++) {
        free(vocab->entries[i].word);
    }
    
    free(vocab->entries);
    free(vocab);
}

bool misaki_ja_vocab_contains(const JaVocab *vocab, const char *word) {
    if (!vocab || !word) {
        return false;
    }
    
    for (int i = 0; i < vocab->count; i++) {
        if (strcmp(vocab->entries[i].word, word) == 0) {
            return true;
        }
    }
    
    return false;
}

void misaki_ja_vocab_stats(const JaVocab *vocab,
                           int *total_words,
                           double *avg_word_length) {
    if (!vocab) {
        return;
    }
    
    if (total_words) {
        *total_words = vocab->count;
    }
    
    if (avg_word_length) {
        double total_len = 0;
        for (int i = 0; i < vocab->count; i++) {
            total_len += strlen(vocab->entries[i].word);
        }
        *avg_word_length = vocab->count > 0 ? total_len / vocab->count : 0;
    }
}

/* ============================================================================
 * 中文词组拼音词典实现
 * ========================================================================== */

ZhPhraseDict* misaki_zh_phrase_dict_load(const char *file_path) {
    if (!file_path) {
        return NULL;
    }
    
    // 创建词组词典对象
    ZhPhraseDict *dict = (ZhPhraseDict *)calloc(1, sizeof(ZhPhraseDict));
    if (!dict) {
        return NULL;
    }
    
    // 创建 Trie 树
    dict->phrase_trie = misaki_trie_create();
    if (!dict->phrase_trie) {
        free(dict);
        return NULL;
    }
    
    dict->count = 0;
    
    // 打开文件
    FILE *f = fopen(file_path, "r");
    if (!f) {
        misaki_trie_free(dict->phrase_trie);
        free(dict);
        return NULL;
    }
    
    // 逐行读取：词<Tab>拼音
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        // 移除换行符
        line[strcspn(line, "\n")] = 0;
        
        // 查找 Tab 分隔符
        char *tab = strchr(line, '\t');
        if (!tab) {
            continue;  // 跳过格式错误的行
        }
        
        // 分割词和拼音
        *tab = '\0';
        char *phrase = line;
        char *pinyin = tab + 1;
        
        // 插入 Trie（将拼音存储在 tag 字段）
        if (misaki_trie_insert(dict->phrase_trie, phrase, 1.0, pinyin)) {
            dict->count++;
        }
    }
    
    fclose(f);
    return dict;
}

void misaki_zh_phrase_dict_free(ZhPhraseDict *dict) {
    if (!dict) {
        return;
    }
    
    if (dict->phrase_trie) {
        misaki_trie_free(dict->phrase_trie);
    }
    
    free(dict);
}

bool misaki_zh_phrase_dict_lookup(const ZhPhraseDict *dict,
                                  const char *phrase,
                                  const char **pinyins) {
    if (!dict || !phrase || !pinyins) {
        return false;
    }
    
    // 查询 Trie 树
    TrieMatch match;
    if (misaki_trie_match_longest(dict->phrase_trie, phrase, 0, &match)) {
        // 找到，返回 tag（拼音字符串）
        *pinyins = match.tag;
        return true;
    }
    
    return false;
}

int misaki_zh_phrase_dict_count(const ZhPhraseDict *dict) {
    return dict ? dict->count : 0;
}
