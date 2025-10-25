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
    ${MISAKI_SRC_DIR}/util/tsv_parser.c
)

# 静态库
add_library(misaki_static STATIC ${MISAKI_SOURCES})
target_include_directories(misaki_static PUBLIC ${MISAKI_INCLUDE_DIR})
set_target_properties(misaki_static PROPERTIES OUTPUT_NAME misaki)

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

# 编译信息
message(STATUS "==============================================")
message(STATUS "Misaki C Port - Simple Build")
message(STATUS "==============================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "==============================================")
