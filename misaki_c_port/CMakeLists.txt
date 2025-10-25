# Misaki C Port - CMake Build Configuration
# 
# Misaki C 语言版本构建配置
# 零依赖，只使用 C 标准库
#
# License: MIT

cmake_minimum_required(VERSION 3.15)

project(misaki_c
    VERSION 1.0.0
    DESCRIPTION "Misaki G2P Engine - C Port"
    LANGUAGES C
)

# ============================================================================
# 编译选项
# ============================================================================

# 使用 C11 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 编译器警告选项
if(MSVC)
    # MSVC 编译器
    add_compile_options(/W4 /WX)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    # GCC/Clang 编译器
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-unused-parameter
        -Wno-unused-function
    )
endif()

# 优化选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2)
    else()
        add_compile_options(-O3 -march=native)
    endif()
endif()

# 调试选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(MISAKI_DEBUG)
    if(NOT MSVC)
        add_compile_options(-g -fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()
endif()

# ============================================================================
# 目录结构
# ============================================================================

set(MISAKI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MISAKI_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MISAKI_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extracted_data)

# ============================================================================
# 源文件
# ============================================================================

# 核心模块
set(MISAKI_CORE_SOURCES
    ${MISAKI_SRC_DIR}/core/misaki_string.c
    ${MISAKI_SRC_DIR}/core/misaki_dict.c
    ${MISAKI_SRC_DIR}/core/misaki_trie.c
    ${MISAKI_SRC_DIR}/core/misaki_viterbi.c
    ${MISAKI_SRC_DIR}/core/misaki_cache.c
    ${MISAKI_SRC_DIR}/core/misaki_context.c
)

# 分词模块
set(MISAKI_TOKENIZER_SOURCES
    ${MISAKI_SRC_DIR}/tokenizer/misaki_tokenizer.c
    ${MISAKI_SRC_DIR}/tokenizer/zh_tokenizer.c
    ${MISAKI_SRC_DIR}/tokenizer/ja_tokenizer.c
    ${MISAKI_SRC_DIR}/tokenizer/en_tokenizer.c
    ${MISAKI_SRC_DIR}/tokenizer/dag.c
    ${MISAKI_SRC_DIR}/tokenizer/hmm.c
)

# G2P 模块
set(MISAKI_G2P_SOURCES
    ${MISAKI_SRC_DIR}/g2p/misaki_g2p.c
    ${MISAKI_SRC_DIR}/g2p/en_g2p.c
    ${MISAKI_SRC_DIR}/g2p/zh_g2p.c
    ${MISAKI_SRC_DIR}/g2p/ja_g2p.c
    ${MISAKI_SRC_DIR}/g2p/ko_g2p.c
    ${MISAKI_SRC_DIR}/g2p/vi_g2p.c
    ${MISAKI_SRC_DIR}/g2p/normalize.c
)

# 工具模块
set(MISAKI_UTIL_SOURCES
    ${MISAKI_SRC_DIR}/util/tsv_parser.c
    ${MISAKI_SRC_DIR}/util/utf8.c
    ${MISAKI_SRC_DIR}/util/logger.c
)

# 所有源文件
set(MISAKI_SOURCES
    ${MISAKI_CORE_SOURCES}
    ${MISAKI_TOKENIZER_SOURCES}
    ${MISAKI_G2P_SOURCES}
    ${MISAKI_UTIL_SOURCES}
)

# ============================================================================
# 库目标
# ============================================================================

# 静态库
add_library(misaki_static STATIC ${MISAKI_SOURCES})
target_include_directories(misaki_static PUBLIC ${MISAKI_INCLUDE_DIR})
set_target_properties(misaki_static PROPERTIES OUTPUT_NAME misaki)

# 动态库（可选）
option(BUILD_SHARED_LIBS "Build shared library" OFF)
if(BUILD_SHARED_LIBS)
    add_library(misaki_shared SHARED ${MISAKI_SOURCES})
    target_include_directories(misaki_shared PUBLIC ${MISAKI_INCLUDE_DIR})
    set_target_properties(misaki_shared PROPERTIES OUTPUT_NAME misaki)
endif()

# ============================================================================
# 测试程序
# ============================================================================

option(BUILD_TESTS "Build test programs" ON)
if(BUILD_TESTS)
    # 字符串测试
    add_executable(test_string tests/test_string.c)
    target_link_libraries(test_string misaki_static)
    
    # 词典测试
    add_executable(test_dict tests/test_dict.c)
    target_link_libraries(test_dict misaki_static)
    
    # Trie 树测试
    add_executable(test_trie tests/test_trie.c)
    target_link_libraries(test_trie misaki_static)
    
    # 分词测试
    add_executable(test_tokenizer tests/test_tokenizer.c)
    target_link_libraries(test_tokenizer misaki_static)
    
    # G2P 测试
    add_executable(test_g2p tests/test_g2p.c)
    target_link_libraries(test_g2p misaki_static)
    
    # 完整测试
    add_executable(test_all tests/test_all.c)
    target_link_libraries(test_all misaki_static)
endif()

# ============================================================================
# 示例程序
# ============================================================================

option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    # 简单示例
    add_executable(example_simple examples/simple.c)
    target_link_libraries(example_simple misaki_static)
    
    # 英文 G2P 示例
    add_executable(example_en examples/english_g2p.c)
    target_link_libraries(example_en misaki_static)
    
    # 中文 G2P 示例
    add_executable(example_zh examples/chinese_g2p.c)
    target_link_libraries(example_zh misaki_static)
    
    # 日文 G2P 示例
    add_executable(example_ja examples/japanese_g2p.c)
    target_link_libraries(example_ja misaki_static)
    
    # 多语言示例
    add_executable(example_multi examples/multilingual.c)
    target_link_libraries(example_multi misaki_static)
endif()

# ============================================================================
# 安装规则
# ============================================================================

# 安装头文件
install(DIRECTORY ${MISAKI_INCLUDE_DIR}/
    DESTINATION include/misaki
    FILES_MATCHING PATTERN "*.h"
)

# 安装库文件
install(TARGETS misaki_static
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

if(BUILD_SHARED_LIBS)
    install(TARGETS misaki_shared
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

# 安装数据文件（可选）
option(INSTALL_DATA "Install dictionary data files" OFF)
if(INSTALL_DATA)
    install(DIRECTORY ${MISAKI_DATA_DIR}/
        DESTINATION share/misaki/data
    )
endif()

# ============================================================================
# 打包配置
# ============================================================================

set(CPACK_PACKAGE_NAME "misaki")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Misaki G2P Engine - C Port")
set(CPACK_PACKAGE_VENDOR "Misaki C Port Project")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)

# ============================================================================
# 编译信息输出
# ============================================================================

message(STATUS "==============================================")
message(STATUS "Misaki C Port - Build Configuration")
message(STATUS "==============================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "Build Shared Libs: ${BUILD_SHARED_LIBS}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "Install Data: ${INSTALL_DATA}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "==============================================")
