#!/bin/bash
# 自动测试交互模式

cd /mnt/e/development/kokoro-ja-demo/misaki_c_port/build

# 使用 heredoc 输入测试命令
./misaki -i << EOF
こんにちは
私は学生です
ありがとうございます
コーヒーを飲みます
test
quit
EOF
