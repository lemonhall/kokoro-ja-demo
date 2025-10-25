#!/bin/bash
# run_all_tests.sh
# 
# 运行所有测试套件
# 

set -e

BUILD_DIR="build"
TEST_PASSED=0
TEST_FAILED=0

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "════════════════════════════════════════════════════════════"
echo "  Misaki C Port - 测试套件"
echo "════════════════════════════════════════════════════════════"

cd "$BUILD_DIR" || exit 1

# 测试列表
TESTS=(
    "test_string:字符串工具"
    "test_tsv:TSV 解析器"
    "test_trie:Trie 数据结构"
    "test_dict:词典加载"
    "test_tokenizer_zh:中文分词器"
    "test_tokenizer_en:英文分词器"
    "test_tokenizer_ja:日文分词器"
    "test_g2p_zh:中文 G2P"
    "test_g2p_en:英文 G2P"
    "test_g2p_ja:日文 G2P"
)

# 运行测试
for test_item in "${TESTS[@]}"; do
    IFS=':' read -r test_name test_desc <<< "$test_item"
    
    if [ ! -f "./$test_name" ]; then
        echo -e "${YELLOW}⚠️  跳过: $test_desc ($test_name 不存在)${NC}"
        continue
    fi
    
    echo ""
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}🧪 运行测试: $test_desc${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    
    if ./"$test_name"; then
        echo -e "${GREEN}✅ $test_desc 测试通过${NC}"
        ((TEST_PASSED++))
    else
        echo -e "${RED}❌ $test_desc 测试失败${NC}"
        ((TEST_FAILED++))
    fi
done

# 总结
echo ""
echo "════════════════════════════════════════════════════════════"
echo "  测试总结"
echo "════════════════════════════════════════════════════════════"
echo -e "  ${GREEN}✅ 通过: $TEST_PASSED${NC}"
echo -e "  ${RED}❌ 失败: $TEST_FAILED${NC}"
echo "════════════════════════════════════════════════════════════"

if [ $TEST_FAILED -eq 0 ]; then
    echo -e "${GREEN}🎉 所有测试通过！${NC}"
    exit 0
else
    echo -e "${RED}⚠️  有测试失败，请检查！${NC}"
    exit 1
fi
