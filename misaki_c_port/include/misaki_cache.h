/**
 * misaki_cache.h
 * 
 * Misaki C Port - LRU Cache
 * LRU 缓存实现（用于缓存分词结果、G2P 结果）
 * 
 * License: MIT
 */

#ifndef MISAKI_CACHE_H
#define MISAKI_CACHE_H

#include "misaki_types.h"
#include "misaki_tokenizer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * LRU Cache 数据结构
 * ========================================================================== */

/**
 * Cache Entry: 缓存条目
 */
typedef struct CacheEntry {
    char *key;                 // 键（文本）
    void *value;               // 值（TokenList 或其他）
    size_t value_size;         // 值大小（字节）
    
    struct CacheEntry *prev;   // 双向链表：前驱
    struct CacheEntry *next;   // 双向链表：后继
    
    uint64_t access_time;      // 最后访问时间
    int access_count;          // 访问次数
} CacheEntry;

/**
 * LRU Cache: 最近最少使用缓存
 */
typedef struct LRUCache {
    CacheEntry **table;        // 哈希表
    int table_size;            // 哈希表大小
    
    CacheEntry *head;          // 链表头（最近使用）
    CacheEntry *tail;          // 链表尾（最久未用）
    
    int count;                 // 当前条目数
    int capacity;              // 容量
    size_t memory_limit;       // 内存限制（字节）
    size_t memory_used;        // 已用内存
    
    // 统计信息
    uint64_t hit_count;        // 命中次数
    uint64_t miss_count;       // 未命中次数
} LRUCache;

/* ============================================================================
 * Cache 操作
 * ========================================================================== */

/**
 * 创建 LRU Cache
 * 
 * @param capacity 容量（条目数）
 * @param memory_limit 内存限制（字节，0 表示无限制）
 * @return Cache 对象，失败返回 NULL
 */
LRUCache* misaki_cache_create(int capacity, size_t memory_limit);

/**
 * 释放 Cache
 * 
 * @param cache Cache 对象
 * @param value_free 值释放函数（可为 NULL）
 */
void misaki_cache_free(LRUCache *cache, void (*value_free)(void*));

/**
 * 插入/更新缓存
 * 
 * @param cache Cache 对象
 * @param key 键（会被复制）
 * @param value 值（会被复制或存储指针，取决于 copy）
 * @param value_size 值大小（字节）
 * @param copy 是否复制值（false 表示直接存储指针）
 * @return 成功返回 true
 */
bool misaki_cache_put(LRUCache *cache,
                      const char *key,
                      void *value,
                      size_t value_size,
                      bool copy);

/**
 * 查询缓存
 * 
 * @param cache Cache 对象
 * @param key 键
 * @return 值指针，未找到返回 NULL
 */
void* misaki_cache_get(LRUCache *cache, const char *key);

/**
 * 删除缓存条目
 * 
 * @param cache Cache 对象
 * @param key 键
 * @param value_free 值释放函数（可为 NULL）
 * @return 成功返回 true
 */
bool misaki_cache_remove(LRUCache *cache,
                         const char *key,
                         void (*value_free)(void*));

/**
 * 清空缓存
 * 
 * @param cache Cache 对象
 * @param value_free 值释放函数（可为 NULL）
 */
void misaki_cache_clear(LRUCache *cache, void (*value_free)(void*));

/**
 * 检查键是否存在
 * 
 * @param cache Cache 对象
 * @param key 键
 * @return 存在返回 true
 */
bool misaki_cache_contains(const LRUCache *cache, const char *key);

/* ============================================================================
 * 专用 Cache（封装 LRU Cache）
 * ========================================================================== */

/**
 * 分词结果缓存
 */
typedef struct TokenizerCache {
    LRUCache *cache;
} TokenizerCache;

/**
 * 创建分词结果缓存
 * 
 * @param capacity 容量
 * @return Cache 对象
 */
TokenizerCache* misaki_tokenizer_cache_create(int capacity);

/**
 * 释放分词结果缓存
 * 
 * @param cache Cache 对象
 */
void misaki_tokenizer_cache_free(TokenizerCache *cache);

/**
 * 插入分词结果
 * 
 * @param cache Cache 对象
 * @param text 文本
 * @param tokens Token 列表（会被复制）
 * @return 成功返回 true
 */
bool misaki_tokenizer_cache_put(TokenizerCache *cache,
                                 const char *text,
                                 const MisakiTokenList *tokens);

/**
 * 查询分词结果
 * 
 * @param cache Cache 对象
 * @param text 文本
 * @return Token 列表（新副本），未找到返回 NULL
 */
MisakiTokenList* misaki_tokenizer_cache_get(TokenizerCache *cache,
                                             const char *text);

/**
 * G2P 结果缓存
 */
typedef struct G2PCache {
    LRUCache *cache;
} G2PCache;

/**
 * 创建 G2P 结果缓存
 * 
 * @param capacity 容量
 * @return Cache 对象
 */
G2PCache* misaki_g2p_cache_create(int capacity);

/**
 * 释放 G2P 结果缓存
 * 
 * @param cache Cache 对象
 */
void misaki_g2p_cache_free(G2PCache *cache);

/**
 * 插入 G2P 结果
 * 
 * @param cache Cache 对象
 * @param text 文本
 * @param phonemes 音素字符串（会被复制）
 * @return 成功返回 true
 */
bool misaki_g2p_cache_put(G2PCache *cache,
                          const char *text,
                          const char *phonemes);

/**
 * 查询 G2P 结果
 * 
 * @param cache Cache 对象
 * @param text 文本
 * @return 音素字符串（新副本），未找到返回 NULL
 */
char* misaki_g2p_cache_get(G2PCache *cache, const char *text);

/* ============================================================================
 * 缓存统计
 * ========================================================================== */

/**
 * 缓存统计信息
 */
typedef struct CacheStats {
    int count;                 // 当前条目数
    int capacity;              // 容量
    size_t memory_used;        // 已用内存
    size_t memory_limit;       // 内存限制
    
    uint64_t hit_count;        // 命中次数
    uint64_t miss_count;       // 未命中次数
    double hit_rate;           // 命中率
    
    int avg_key_size;          // 平均键大小
    int avg_value_size;        // 平均值大小
} CacheStats;

/**
 * 获取缓存统计信息
 * 
 * @param cache Cache 对象
 * @param stats 输出：统计信息
 */
void misaki_cache_get_stats(const LRUCache *cache, CacheStats *stats);

/**
 * 打印缓存统计信息
 * 
 * @param cache Cache 对象
 */
void misaki_cache_print_stats(const LRUCache *cache);

/**
 * 重置统计计数器
 * 
 * @param cache Cache 对象
 */
void misaki_cache_reset_stats(LRUCache *cache);

/* ============================================================================
 * 缓存策略配置
 * ========================================================================== */

/**
 * 缓存淘汰策略
 */
typedef enum {
    CACHE_EVICT_LRU,           // 最近最少使用
    CACHE_EVICT_LFU,           // 最不经常使用
    CACHE_EVICT_FIFO,          // 先进先出
} CacheEvictPolicy;

/**
 * 设置淘汰策略
 * 
 * @param cache Cache 对象
 * @param policy 淘汰策略
 */
void misaki_cache_set_evict_policy(LRUCache *cache, CacheEvictPolicy policy);

/**
 * 设置过期时间
 * 
 * @param cache Cache 对象
 * @param ttl_seconds 过期时间（秒，0 表示永不过期）
 */
void misaki_cache_set_ttl(LRUCache *cache, int ttl_seconds);

/**
 * 手动触发过期清理
 * 
 * @param cache Cache 对象
 * @param value_free 值释放函数（可为 NULL）
 * @return 清理的条目数
 */
int misaki_cache_expire(LRUCache *cache, void (*value_free)(void*));

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_CACHE_H */
