#include "include/misaki_num2cn.h"
#include <stdio.h>

int main() {
    char buffer[256];
    
    printf("=== 测试数字转换 ===\n");
    
    misaki_int_to_chinese(1500000, buffer, sizeof(buffer), true);
    printf("1500000 → [%s]\n", buffer);
    
    misaki_int_to_chinese(123, buffer, sizeof(buffer), true);
    printf("123 → [%s]\n", buffer);
    
    misaki_float_to_chinese(3.14, buffer, sizeof(buffer), true);
    printf("3.14 → [%s]\n", buffer);
    
    printf("\n=== 测试文本转换 ===\n");
    
    char text_buffer[512];
    misaki_convert_numbers_in_text("我有1,500,000元", text_buffer, sizeof(text_buffer));
    printf("我有1,500,000元 → [%s]\n", text_buffer);
    
    misaki_convert_numbers_in_text("价格是12.5%", text_buffer, sizeof(text_buffer));
    printf("价格是12.5%% → [%s]\n", text_buffer);
    
    return 0;
}
