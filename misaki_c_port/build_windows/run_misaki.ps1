# run_misaki.ps1
# Windows PowerShell 下运行 misaki.exe 的启动脚本
# 解决 UTF-8 中文显示问题

# 设置控制台输出编码为 UTF-8
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

# 设置代码页为 UTF-8 (65001)
chcp 65001 > $null

Write-Host "✅ 已设置 UTF-8 编码" -ForegroundColor Green
Write-Host ""

# 运行 misaki.exe，传递所有参数
& ".\misaki.exe" $args
