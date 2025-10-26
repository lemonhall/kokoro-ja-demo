/**
 * misaki_api.h
 * 
 * Misaki G2P - 导出 API
 * 用于 Python/Kotlin/Swift 等语言调用
 * 
 * License: MIT
 */

#ifndef MISAKI_API_H
#define MISAKI_API_H

#ifdef __cplusplus
extern "C" {
#endif

// Windows DLL 导出声明
#ifdef _WIN32
#ifdef MISAKI_BUILD_DLL
#define MISAKI_API __declspec(dllexport)
#else
#define MISAKI_API __declspec(dllimport)
#endif
#else
#define MISAKI_API
#endif

/**
 * 初始化 Misaki G2P 引擎
 * 
 * @param data_dir 数据目录路径（如 "../extracted_data"）
 * @return 0=成功, -1=失败
 */
MISAKI_API int misaki_init(const char *data_dir);

/**
 * 文本转音素（自动检测语言）
 * 
 * @param text 输入文本（UTF-8）
 * @param output_buffer 输出缓冲区（调用者分配）
 * @param buffer_size 缓冲区大小
 * @return 0=成功, -1=失败
 */
MISAKI_API int misaki_text_to_phonemes(
    const char *text,
    char *output_buffer,
    int buffer_size
);

/**
 * 文本转音素（指定语言）
 * 
 * @param text 输入文本（UTF-8）
 * @param lang 语言代码（"ja"=日文, "zh"=中文, "en"=英文）
 * @param output_buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 0=成功, -1=失败
 */
MISAKI_API int misaki_text_to_phonemes_lang(
    const char *text,
    const char *lang,
    char *output_buffer,
    int buffer_size
);

/**
 * 清理 Misaki G2P 引擎
 */
MISAKI_API void misaki_cleanup(void);

/**
 * 获取版本号
 * 
 * @return 版本字符串（如 "0.3.0"）
 */
MISAKI_API const char* misaki_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_API_H */
