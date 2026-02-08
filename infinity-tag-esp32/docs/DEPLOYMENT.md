# 🚀 部署和运维指南

本文档提供 Infinity Tag ESP32 固件的部署流程、监控方法和故障排查指南。

---

## 📋 目录

- [部署流程](#部署流程)
- [固件版本管理](#固件版本管理)
- [OTA 更新部署](#ota-更新部署)
- [监控和日志](#监控和日志)
- [常见问题和修复](#常见问题和修复)
- [回滚流程](#回滚流程)
- [性能优化](#性能优化)

---

## 🔧 部署流程

### 首次部署（USB 串口）

#### 1. 准备工作

```bash
# 检查环境
pio --version
python --version

# 克隆项目
git clone <repository-url>
cd infinity-tag-esp32

# 安装依赖
pio lib install
```

#### 2. 配置固件

**编辑 `include/Version.h`**：

```cpp
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_BUILD 12
```

**配置调试日志**（可选）：

```cpp
// include/Utils/Logger.h
// 生产环境建议注释掉
// #define ENABLE_DEBUG_LOGGING
```

#### 3. 编译固件

```bash
# 清理旧的编译文件
pio run -t clean

# 编译固件
pio run

# 检查编译输出
# RAM:   [====      ]  45.2% (used 148256 bytes from 327680 bytes)
# Flash: [======    ]  62.8% (used 1036288 bytes from 1650000 bytes)
```

#### 4. 上传固件

```bash
# 连接设备到 USB
# 查看可用串口
pio device list

# 上传固件
pio run -t upload

# 如果失败，手动指定串口
pio run -t upload --upload-port COM3  # Windows
pio run -t upload --upload-port /dev/ttyUSB0  # Linux
```

#### 5. 上传文件系统（首次部署）

```bash
# 准备文件系统数据
# 将图标、Lua 脚本等放入 data/ 目录

# 上传文件系统
pio run -t uploadfs
```

#### 6. 验证部署

```bash
# 打开串口监视器
pio device monitor

# 检查启动日志
# [System] Infinity Tag ESP32 v1.0.0.12
# [System] Free heap: 245760 bytes
# [WiFi] Starting provisioning...
```

---

## 📦 固件版本管理

### 版本号规范

格式：`vMAJOR.MINOR.PATCH.BUILD`

| 字段 | 说明 | 何时递增 |
|------|------|----------|
| **MAJOR** | 主版本号 | 重大架构变更、不兼容更新 |
| **MINOR** | 次版本号 | 新功能添加、向后兼容 |
| **PATCH** | 修订号 | Bug 修复、小改进 |
| **BUILD** | 构建号 | 每次构建自动递增 |

### 版本更新流程

```bash
# 1. 更新版本号
# 编辑 include/Version.h

# 2. 提交版本更新
git add include/Version.h
git commit -m "chore: bump version to v1.0.1.0"

# 3. 创建 Git 标签
git tag -a v1.0.1.0 -m "Release v1.0.1.0"
git push origin v1.0.1.0

# 4. 编译发布版本
pio run -e release  # 如果有 release 环境配置
```

### 固件打包

```bash
# 编译后的固件位置
.pio/build/4d_systems_esp32s3_gen4_r8n16/firmware.bin

# 重命名为版本号
cp .pio/build/4d_systems_esp32s3_gen4_r8n16/firmware.bin \
   releases/infinity-tag-v1.0.1.0.bin

# 计算 SHA256（用于 OTA 验证）
sha256sum releases/infinity-tag-v1.0.1.0.bin
```

---

## 🌐 OTA 更新部署

### 后端准备

#### 1. 上传固件到后端

```bash
# 使用后端 API 上传固件
curl -X POST http://your-backend:8001/api/admin/firmware/upload \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -F "file=@releases/infinity-tag-v1.0.1.0.bin" \
  -F "version=1.0.1.0" \
  -F "description=修复墨水屏残影问题"
```

#### 2. 验证固件上传

```bash
# 检查固件列表
curl http://your-backend:8001/api/firmware/latest \
  -H "Authorization: Bearer DEVICE_TOKEN"

# 响应示例
{
  "version": "1.0.1.0",
  "size": 1036288,
  "url": "http://your-backend:8001/api/firmware/download/1.0.1.0",
  "sha256": "abc123...",
  "release_notes": "修复墨水屏残影问题"
}
```

### 设备端 OTA 更新

#### 方法 A：设备主动检查更新

设备会在以下情况检查更新：
1. 启动时自动检查
2. 用户在设置菜单中选择"检查更新"
3. 定时检查（如果配置）

#### 方法 B：后端推送更新

```bash
# 推送更新到指定设备
curl -X POST http://your-backend:8001/api/admin/ota/push \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "ESP32-XXXXXX",
    "version": "1.0.1.0",
    "force": false
  }'
```

### OTA 更新流程

设备端 OTA 更新流程：

```
1. 检查更新
   ↓
2. 显示更新信息（版本、大小）
   ↓
3. 用户确认
   ↓
4. 下载固件（显示进度条）
   ↓
5. 验证 SHA256
   ↓
6. 写入 Flash
   ↓
7. 重启设备
   ↓
8. 验证新版本
```

### OTA 更新监控

```bash
# 查看设备 OTA 状态
curl http://your-backend:8001/api/admin/devices/ESP32-XXXXXX/ota-status \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN"

# 响应示例
{
  "device_id": "ESP32-XXXXXX",
  "current_version": "1.0.0.12",
  "target_version": "1.0.1.0",
  "status": "downloading",
  "progress": 45,
  "last_update": "2026-02-08T10:30:00Z"
}
```

---

## 📊 监控和日志

### 串口日志监控

#### 启动日志

```
[System] Infinity Tag ESP32 v1.0.0.12
[System] ESP32-S3 @ 240MHz
[System] Free heap: 245760 bytes
[System] PSRAM: 8388608 bytes
[System] Flash: 16777216 bytes

[EPD] Initializing display...
[EPD] Display initialized

[WiFi] Loading config from NVS...
[WiFi] SSID: MyWiFi
[WiFi] Connecting...
[WiFi] Connected! IP: 192.168.1.100

[Backend] Authenticating...
[Backend] JWT token received

[Cards] Loading cards...
[Cards] Registered 5 cards

[System] Ready!
```

#### 运行时日志

```
[InputManager] Button pressed
[CardManager] Switching to card 1
[EPD] Partial refresh (1/5)
[Card] Rendering weather card

[Network] Fetching image from backend...
[Network] Downloaded 2808 bytes
[EPD] Full refresh

[OTA] Checking for updates...
[OTA] New version available: v1.0.1.0
[OTA] User confirmed update
[OTA] Downloading... 45%
```

### 内存监控

```cpp
// 在关键位置添加内存监控
void checkMemory() {
    Serial.printf("[Memory] Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("[Memory] PSRAM free: %d bytes\n", ESP.getFreePsram());
    Serial.printf("[Memory] Largest free block: %d bytes\n",
                  ESP.getMaxAllocHeap());
}
```

### 性能监控

```cpp
// 测量关键操作耗时
unsigned long start = millis();
epd.refreshFull(drawFunc);
unsigned long elapsed = millis() - start;
LOG_PRINTF("[Performance] Full refresh took %lu ms\n", elapsed);
```

### 看门狗监控

```cpp
// 配置看门狗（30秒超时）
esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 30000,
    .idle_core_mask = (1 << 0) | (1 << 1),
    .trigger_panic = true
};
esp_task_wdt_init(&wdt_config);

// 在主循环中喂狗
void loop() {
    esp_task_wdt_reset();
    // ... 其他代码
}
```

---

## 🐛 常见问题和修复

### 问题 1：设备无法启动

**症状**：
- 串口无输出
- 屏幕无显示
- LED 不亮

**排查步骤**：

```bash
# 1. 检查电源
# - 电池电压是否足够（>3.3V）
# - USB 供电是否正常

# 2. 检查固件
pio device monitor
# 如果无输出，重新上传固件
pio run -t upload

# 3. 检查硬件连接
# - 墨水屏排线是否松动
# - 按键是否卡死
```

**解决方案**：
- 更换电池或使用 USB 供电
- 重新上传固件
- 检查硬件连接

---

### 问题 2：WiFi 连接失败

**症状**：
```
[WiFi] Connecting...
[WiFi] Connection failed!
[WiFi] Starting provisioning...
```

**排查步骤**：

```bash
# 1. 检查 WiFi 配置
# 连接设备热点 InfinityTag-XXXXXX
# 查看配置是否正确

# 2. 检查 WiFi 信号强度
# 在串口输出中查找 RSSI 值
# RSSI > -70 dBm 为良好

# 3. 清除 WiFi 配置
# 在设置菜单中选择"恢复出厂"
# 或者通过串口命令清除 NVS
```

**解决方案**：
- 重新配置 WiFi
- 移动设备到信号更好的位置
- 检查路由器设置（是否禁用了 2.4GHz）

---

### 问题 3：墨水屏残影严重

**症状**：
- 切换卡片后有明显残影
- 文字重叠显示

**排查步骤**：

```bash
# 1. 检查刷新模式
# 在串口输出中查找刷新日志
[EPD] Partial refresh (5/5)
[EPD] Auto flicker (5 partial reached)

# 2. 检查刷新计数器
# 确认每 5 次局部刷新会触发闪烁刷新
```

**解决方案**：
- 手动触发全屏刷新（长按按键）
- 调整自动闪烁阈值（`PARTIAL_REFRESH_THRESHOLD`）
- 使用深度刷新模式

---

### 问题 4：OTA 更新失败

**症状**：
```
[OTA] Downloading... 45%
[OTA] Download failed! HTTP code: 404
```

**排查步骤**：

```bash
# 1. 检查后端服务
curl http://your-backend:8001/api/firmware/latest

# 2. 检查网络连接
# 在串口输出中查找网络状态

# 3. 检查固件大小
# 确保固件不超过分区大小（约 1.6MB）
```

**解决方案**：
- 检查后端服务是否正常
- 检查固件 URL 是否正确
- 减小固件大小（移除调试代码）

---

### 问题 5：内存不足崩溃

**症状**：
```
[Memory] Free heap: 12560 bytes
Guru Meditation Error: Core 0 panic'ed (LoadProhibited)
```

**排查步骤**：

```bash
# 1. 查看内存使用
# 在串口输出中查找 "Free heap" 信息

# 2. 检查内存泄漏
# 运行一段时间后，内存是否持续下降

# 3. 使用 ESP32 Exception Decoder
# 分析崩溃堆栈
```

**解决方案**：
- 使用智能指针管理内存
- 减少全局变量
- 使用 PSRAM 存储大数据
- 及时释放不用的资源

---

### 问题 6：按键无响应

**症状**：
- 按键按下无反应
- 旋转编码器不工作

**排查步骤**：

```bash
# 1. 检查输入日志
[InputManager] Button pressed
[InputManager] Encoder rotated: 1

# 2. 检查引脚配置
# 确认 PinConfig.h 中的引脚定义正确

# 3. 检查硬件
# 使用万用表测试按键是否正常
```

**解决方案**：
- 检查引脚配置
- 检查硬件连接
- 调整防抖参数

---

## 🔄 回滚流程

### 场景 1：OTA 更新后设备异常

#### 自动回滚

ESP32 支持双分区 OTA，更新失败会自动回滚：

```cpp
// 设备启动时验证固件
if (esp_ota_get_boot_partition() != esp_ota_get_running_partition()) {
    // 新固件启动失败，自动回滚到旧版本
    esp_ota_mark_app_invalid_rollback_and_reboot();
}
```

#### 手动回滚

```bash
# 方法 A：通过后端推送旧版本
curl -X POST http://your-backend:8001/api/admin/ota/push \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "ESP32-XXXXXX",
    "version": "1.0.0.12",
    "force": true
  }'

# 方法 B：USB 串口上传旧固件
pio run -t upload
```

---

### 场景 2：批量设备回滚

```bash
# 1. 准备回滚脚本
cat > rollback.sh << 'EOF'
#!/bin/bash
BACKEND="http://your-backend:8001"
TOKEN="YOUR_ADMIN_TOKEN"
OLD_VERSION="1.0.0.12"

# 获取所有设备列表
DEVICES=$(curl -s "$BACKEND/api/admin/devices" \
  -H "Authorization: Bearer $TOKEN" | jq -r '.[].device_id')

# 逐个回滚
for DEVICE in $DEVICES; do
  echo "Rolling back $DEVICE to $OLD_VERSION..."
  curl -X POST "$BACKEND/api/admin/ota/push" \
    -H "Authorization: Bearer $TOKEN" \
    -H "Content-Type: application/json" \
    -d "{
      \"device_id\": \"$DEVICE\",
      \"version\": \"$OLD_VERSION\",
      \"force\": true
    }"
  sleep 5
done
EOF

# 2. 执行回滚
chmod +x rollback.sh
./rollback.sh
```

---

## ⚡ 性能优化

### 编译优化

```ini
# platformio.ini
[env:release]
platform = espressif32
board = 4d_systems_esp32s3_gen4_r8n16
framework = arduino

build_flags =
    -DBOARD_HAS_PSRAM
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DCORE_DEBUG_LEVEL=0           # 禁用调试日志
    -Os                             # 优化代码大小
    -DNDEBUG                        # 禁用断言

# 禁用调试日志
build_unflags =
    -DENABLE_DEBUG_LOGGING
```

### 内存优化

```cpp
// 1. 使用 PSRAM 存储大数据
uint8_t* buffer = (uint8_t*)ps_malloc(2808);

// 2. 及时释放资源
{
    std::unique_ptr<Image> img = loadImage();
    processImage(img.get());
}  // img 自动释放

// 3. 使用常量字符串
const char* MENU_TEXTS[] PROGMEM = {
    "网络设置",
    "声音",
    "固件信息"
};
```

### 刷新优化

```cpp
// 1. 优先使用局部刷新
epd.refreshPartial(drawFunc);

// 2. 批量更新后再刷新
epd.refreshFull([](EPD_Class &d) {
    // 一次性绘制所有内容
    drawTitle(d);
    drawMenu(d);
    drawStatusBar(d);
});

// 3. 避免频繁全屏刷新
// 每 5 次局部刷新才触发一次闪烁刷新
```

---

## 📚 参考资源

### 官方文档
- [ESP32 OTA 更新](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ota.html)
- [PlatformIO 部署](https://docs.platformio.org/en/latest/core/userguide/cmd_run.html)

### 项目文档
- [架构设计](./ARCHITECTURE.md)
- [开发指南](./CONTRIBUTING.md)
- [调试指南](./DEBUG_GUIDE.md)

---

**文档版本**: v1.0
**最后更新**: 2026-02-08
**维护者**: Infinity Tag Team

---

💡 **提示**: 部署前建议先在测试设备上验证，确保固件稳定后再批量部署！
