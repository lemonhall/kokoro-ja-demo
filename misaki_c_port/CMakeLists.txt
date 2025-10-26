# Misaki C Port - 简化构建配置（仅编译现有文件）
cmake_minimum_required(VERSION 3.15)

project(misaki_c
    VERSION 1.0.0
    DESCRIPTION "Misaki G2P Engine - C Port"
    LANGUAGES C
)

# C11 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -g)
endif()

# 目录
set(MISAKI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MISAKI_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 源文件（仅现有文件）
set(MISAKI_SOURCES
    ${MISAKI_SRC_DIR}/core/misaki_string.c
    ${MISAKI_SRC_DIR}/core/misaki_dict.c
    ${MISAKI_SRC_DIR}/core/misaki_trie.c
    ${MISAKI_SRC_DIR}/core/misaki_viterbi.c
    ${MISAKI_SRC_DIR}/core/misaki_hmm.c  # 新增：中文 HMM 未登录词识别
    ${MISAKI_SRC_DIR}/core/misaki_tokenizer.c
    ${MISAKI_SRC_DIR}/core/misaki_tokenizer_zh.c
    ${MISAKI_SRC_DIR}/core/misaki_tokenizer_en.c
    ${MISAKI_SRC_DIR}/core/misaki_tokenizer_ja.c
    ${MISAKI_SRC_DIR}/core/misaki_transition_rules.c  # 新增：词性转移规则
    ${MISAKI_SRC_DIR}/core/misaki_g2p.c
    ${MISAKI_SRC_DIR}/core/misaki_g2p_zh.c
    ${MISAKI_SRC_DIR}/core/misaki_g2p_en.c
    ${MISAKI_SRC_DIR}/core/misaki_g2p_ja.c
    ${MISAKI_SRC_DIR}/core/misaki_kana_map.c
    ${MISAKI_SRC_DIR}/util/tsv_parser.c
)

# 静态库
add_library(misaki_static STATIC ${MISAKI_SOURCES})
target_include_directories(misaki_static PUBLIC ${MISAKI_INCLUDE_DIR})
set_target_properties(misaki_static PROPERTIES OUTPUT_NAME misaki)

# 命令行工具（主程序）
add_executable(misaki src/main.c)
target_link_libraries(misaki misaki_static m)

# 测试程序
add_executable(test_string tests/test_string.c)
target_link_libraries(test_string misaki_static)

add_executable(test_tsv tests/test_tsv.c)
target_link_libraries(test_tsv misaki_static)

add_executable(test_dict tests/test_dict.c)
target_link_libraries(test_dict misaki_static)

add_executable(test_trie tests/test_trie.c)
target_link_libraries(test_trie misaki_static)

add_executable(test_viterbi tests/test_viterbi.c)
target_link_libraries(test_viterbi misaki_static m)

add_executable(test_tokenizer tests/test_tokenizer.c)
target_link_libraries(test_tokenizer misaki_static m)

add_executable(test_zh_tokenizer_real tests/test_zh_tokenizer_real.c)
target_link_libraries(test_zh_tokenizer_real misaki_static m)

add_executable(test_g2p tests/test_g2p.c)
target_link_libraries(test_g2p misaki_static m)

# 新增：按语言拆分的测试
add_executable(test_tokenizer_zh tests/test_tokenizer_zh.c)
target_link_libraries(test_tokenizer_zh misaki_static m)

add_executable(test_tokenizer_en tests/test_tokenizer_en.c)
target_link_libraries(test_tokenizer_en misaki_static m)

add_executable(test_tokenizer_ja tests/test_tokenizer_ja.c)
target_link_libraries(test_tokenizer_ja misaki_static m)

add_executable(test_g2p_zh tests/test_g2p_zh.c)
target_link_libraries(test_g2p_zh misaki_static m)

add_executable(test_g2p_en tests/test_g2p_en.c)
target_link_libraries(test_g2p_en misaki_static m)

add_executable(test_g2p_ja tests/test_g2p_ja.c)
target_link_libraries(test_g2p_ja misaki_static m)

# 完整测试套件
add_executable(test_ja_comprehensive tests/test_ja_comprehensive.c)
target_link_libraries(test_ja_comprehensive misaki_static m)

# 日文读音词典测试
add_executable(test_ja_pron_dict tests/test_ja_pron_dict.c)
target_link_libraries(test_ja_pron_dict misaki_static m)

# 日文完整 G2P 测试
add_executable(test_ja_g2p_full tests/test_ja_g2p_full.c)
target_link_libraries(test_ja_g2p_full misaki_static m)

# 假名转换调试
add_executable(test_kana_debug tests/test_kana_debug.c)
target_link_libraries(test_kana_debug misaki_static m)

# Trie 匹配测试
add_executable(test_trie_match tests/test_trie_match.c)
target_link_libraries(test_trie_match misaki_static m)

# 编译信息
message(STATUS "==============================================")
message(STATUS "Misaki C Port - Simple Build")
message(STATUS "==============================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "==============================================")
add_executable(test_lookup_debug tests/test_lookup_debug.c)
target_link_libraries(test_lookup_debug misaki_static m)

# 中文大词典测试
add_executable(test_zh_full_dict tests/test_zh_full_dict.c)
target_link_libraries(test_zh_full_dict misaki_static m)

# HMM 未登录词识别测试
add_executable(test_hmm tests/test_hmm.c)
target_link_libraries(test_hmm misaki_static m)
