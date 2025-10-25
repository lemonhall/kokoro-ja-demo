#include "include/misaki_viterbi.h"
#include "include/misaki_trie.h"
#include "include/misaki_string.h"
#include <stdio.h>
#include <math.h>

int main() {
    // 创建 Trie
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "こんにちは", 1000.0, NULL);
    
    // 创建 Lattice
    const char *text = "こんにちは";
    int text_len = misaki_utf8_length(text);
    printf("文本长度: %d 个字符\n", text_len);
    
    Lattice *lattice = misaki_lattice_create(text_len);
    printf("Lattice 创建成功\n");
    
    // 添加节点
    LatticeNode *node = misaki_lattice_add_node(lattice, 0, "こんにちは", NULL, NULL, -log(1000.0));
    printf("节点添加成功: %p, cost=%.2f\n", (void*)node, node->node_cost);
    node->length = text_len;
    
    printf("BOS: %p, cost=%.2f\n", (void*)lattice->bos, lattice->bos->total_cost);
    printf("EOS: %p, cost=%.2f\n", (void*)lattice->eos, lattice->eos->total_cost);
    
    // 连接边：BOS -> node
    misaki_lattice_add_edge(lattice->bos, node, 0.0);
    printf("BOS -> node: next_count=%d\n", lattice->bos->next_count);
    
    // 连接边：node -> EOS
    misaki_lattice_add_edge(node, lattice->eos, 0.0);
    printf("node -> EOS: next_count=%d\n", node->next_count);
    
    // 打印 Lattice 结构
    printf("\nLattice 结构:\n");
    for (int pos = 0; pos <= text_len; pos++) {
        printf("  位置 %d: %d 个节点\n", pos, lattice->node_counts[pos]);
        for (int i = 0; i < lattice->node_counts[pos]; i++) {
            LatticeNode *n = lattice->nodes[pos][i];
            printf("    [%d] %s (cost=%.2f, next_count=%d)\n", 
                   i, n->surface, n->node_cost, n->next_count);
        }
    }
    
    // Viterbi 搜索
    printf("\n执行 Viterbi 搜索...\n");
    bool success = misaki_viterbi_search(lattice);
    printf("Viterbi 结果: %d\n", success);
    
    printf("EOS total_cost: %.2f\n", lattice->eos->total_cost);
    printf("EOS prev: %p\n", (void*)lattice->eos->prev);
    
    if (lattice->eos->prev) {
        printf("EOS.prev = %s\n", lattice->eos->prev->surface);
    }
    
    // 提取路径
    printf("\n提取路径...\n");
    MisakiTokenList *tokens = misaki_viterbi_extract_tokens(lattice);
    printf("Tokens: %p\n", (void*)tokens);
    
    if (tokens) {
        printf("分词数量: %d\n", tokens->count);
        for (int i = 0; i < tokens->count; i++) {
            printf("  [%d] %s\n", i, tokens->tokens[i].text);
        }
        misaki_token_list_free(tokens);
    } else {
        printf("提取失败！\n");
    }
    
    misaki_lattice_free(lattice);
    misaki_trie_free(trie);
    return 0;
}
