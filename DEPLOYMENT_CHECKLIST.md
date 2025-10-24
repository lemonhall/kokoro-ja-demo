# 🚀 Kokoro TTS Android 部署清单

## ✅ 已完成的准备工作

### 1. Python 端导出 ✅
- [x] ONNX 模型导出 (`kokoro_latest.onnx`)
- [x] INT8 量化 (`kokoro_latest_int8.onnx` - 109MB)
- [x] 词汇表导出 (`KokoroVocabFull.kt`)
- [x] 语音嵌入导出 (`jf_nezumi.bin` - 1KB)

### 2. Android 项目文件 ✅
- [x] ONNX Runtime 依赖配置
- [x] 推理引擎 (`KokoroEngine.kt`)
- [x] 语音加载器 (`VoiceEmbeddingLoader.kt`)
- [x] 词汇表 (`KokoroVocabFull.kt`)
- [x] 示例界面 (`MainActivity.kt`)

### 3. Assets 文件 ✅
```
app/src/main/assets/
├── kokoro_latest_int8.onnx  ✅ (109 MB) - INT8 量化模型
└── jf_nezumi.bin            ✅ (1 KB)   - 女声嵌入
```

## 📱 部署到 Android 设备

### 步骤 1: 打开项目
```bash
# 用 Android Studio 打开项目
# File -> Open -> 选择 kokoro-ja-demo 目录
```

### 步骤 2: 同步 Gradle
- Android Studio 会自动提示同步
- 或手动点击 "Sync Project with Gradle Files"

### 步骤 3: 连接设备
- 连接 Android 手机（开启 USB 调试）
- 或启动 Android 模拟器（推荐 API 29+）

### 步骤 4: 运行
- 点击绿色的 Run 按钮 ▶️
- 或按 `Shift + F10`

## 🧪 测试流程

### 预期行为
1. ✅ 应用启动
2. ✅ 显示 "正在加载模型..."
3. ✅ 加载成功后显示 "✅ 模型加载成功"
4. ✅ 点击 "测试语音合成" 按钮
5. ✅ 显示 "正在生成语音..."
6. ✅ 生成成功，显示波形信息
7. ✅ 自动播放日语 "こんにちは"

### 当前测试内容
- **文本**: "こんにちは" (你好)
- **音素**: `koɲɲiʨiβa`
- **语音**: jf_nezumi (女声)
- **预期时长**: ~1.7 秒

## 📊 性能参考

### 设备性能预期

| 设备 | CPU | RAM | 加载时间 | 推理时间 | 备注 |
|------|-----|-----|---------|---------|------|
| Pixel 7 Pro | Tensor G2 | 12GB | ~1s | ~0.5s | NNAPI 加速 |
| OnePlus 9 | SD 888 | 8GB | ~1.5s | ~1s | 良好 |
| Redmi Note 10 | SD 678 | 6GB | ~2s | ~2s | 可接受 |
| 模拟器 (x86) | - | 4GB | ~3s | ~5s | 较慢 |

### 内存占用
- **模型加载**: ~120MB
- **推理时峰值**: ~180MB
- **播放时**: ~150MB

## 🔍 问题排查

### ❌ 模型加载失败
**症状**: "❌ 模型加载失败"

**检查**:
1. APK 中是否包含模型文件
   ```bash
   # 解压 APK 检查
   unzip -l app/build/outputs/apk/debug/app-debug.apk | grep onnx
   ```
2. 文件大小是否正确 (应该是 109MB)
3. 查看 Logcat 错误详情

### ❌ NNAPI 错误
**症状**: "NNAPI not available"

**解决**: 
- 这是正常的，会自动回退到 CPU
- Android 8.1+ 才支持 NNAPI
- 某些模拟器不支持 NNAPI

### ❌ 推理失败
**症状**: "❌ 生成失败"

**检查**:
1. 语音嵌入是否正确加载
2. input_ids 是否正确
3. 查看 Logcat 详细错误

### ❌ 无声音/噪音
**症状**: 播放时无声音或只有噪音

**检查**:
1. ✅ 现在使用真实语音嵌入，应该不会有这个问题
2. 确认 `jf_nezumi.bin` 存在
3. 检查音频权限

## 📦 APK 打包

### Debug 版本
```bash
# 生成 debug APK
./gradlew assembleDebug

# 输出位置
app/build/outputs/apk/debug/app-debug.apk
```

### Release 版本
```bash
# 配置签名后
./gradlew assembleRelease
```

**注意**: Release APK 会启用代码混淆，确保 ONNX Runtime 相关类不被混淆。

## 🎯 下一步优化

### 短期 (1-2天)
- [ ] 添加文本输入框，支持自定义文本
- [ ] 添加语速控制滑块
- [ ] 显示推理耗时统计

### 中期 (1周)
- [ ] 集成 MeCab 进行日文 G2P 转换
- [ ] 支持长文本分句处理
- [ ] 添加音频保存功能

### 长期 (1个月+)
- [ ] 支持多语音切换
- [ ] 实现流式合成
- [ ] 离线缓存优化
- [ ] 模型热更新

## 📝 已知限制

1. **只支持音素输入**: 需要手动将日文转为音素
2. **单一语音**: 当前只有 jf_nezumi
3. **没有 G2P**: 需要外部工具转换文本
4. **INT8 兼容性**: 桌面 ONNX Runtime 可能不支持

## 🔗 相关文档

- [ANDROID_GUIDE.md](ANDROID_GUIDE.md) - 完整集成指南
- [README.md](README.md) - 项目主文档
- [导出到onnx.md](导出到onnx.md) - ONNX 技术文档

---

✅ **准备就绪！现在可以在 Android Studio 中运行项目了！**
