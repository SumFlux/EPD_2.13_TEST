# ESP32 网络图片显示功能 - 配置指南

## 快速开始

### 1. 修改配置参数

打开 `src/main.cpp`，修改以下配置：

```cpp
// WiFi 配置
#define WIFI_SSID "YourWiFiName"          // 修改为你的 WiFi 名称
#define WIFI_PASSWORD "YourPassword"      // 修改为你的 WiFi 密码

// 后端服务器配置
#define API_BASE_URL "http://192.168.1.100:8000"  // 修改为后端服务器地址

// 设备凭证
#define DEVICE_ID "ABC123"                // 修改为你的设备 ID
#define DEVICE_PASSWORD "123456"          // 修改为你的设备密码
```

### 2. 编译和上传

```bash
# 使用 PlatformIO CLI
pio run -t upload

# 或使用 VSCode PlatformIO 插件
# 点击底部状态栏的 "Upload" 按钮
```

### 3. 查看串口输出

```bash
pio device monitor -b 115200
```

## 功能说明

### 启动流程

1. **初始化硬件**：墨水屏、输入管理器
2. **连接 WiFi**：10 秒超时
3. **后端认证**：获取 access_token
4. **显示就绪画面**：提示按按键获取图片

### 操作说明

| 操作 | 功能 |
|------|------|
| **按键按下** | 获取图片列表并显示第一张图片 |
| **旋转编码器** | 切换图片（使用闪烁刷新） |
| **振动开关** | 计数器递增（保留功能） |

### 串口日志示例

```
--- INFINITY TAG V2 (Network) ---
Firmware: v1.0.0.1
Allocating bitmap buffer...
PSRAM detected, using ps_malloc
Bitmap buffer allocated
Connecting to WiFi...........
WiFi Connected!
IP: 192.168.1.123
Authenticating...
Authentication successful!
Token: eyJhbGciOiJIUzI1Ni...
--- SYSTEM READY ---

[BTN] Fetch images
Fetching image list...
Fetched 3 images
Image 0: id=12, url=/assets/custom_images/1/abc123.png
Image 1: id=10, url=/assets/custom_images/1/def456.png
Image 2: id=8, url=/assets/custom_images/1/ghi789.png
Downloading bitmap for image 12...
Downloaded 2808 bytes
Bitmap displayed

[ENC] Switch image: 1 (delta=1)
Downloading bitmap for image 10...
Downloaded 2808 bytes
Bitmap displayed
```

## 错误处理

### WiFi 连接失败

**错误信息**：`WiFi Failed`

**解决方案**：
1. 检查 WiFi SSID 和密码是否正确
2. 确认 WiFi 信号强度
3. 检查路由器是否开启 2.4GHz 频段（ESP32 不支持 5GHz）

### 认证失败

**错误信息**：`Auth Failed`

**解决方案**：
1. 检查 DEVICE_ID 和 DEVICE_PASSWORD 是否正确
2. 确认后端服务器地址是否正确
3. 检查后端服务是否正常运行

### 无图片

**错误信息**：`No Images`

**解决方案**：
1. 登录后端管理界面上传图片
2. 确认图片已分配给当前设备

### 下载失败

**错误信息**：`Download Failed`

**解决方案**：
1. 检查网络连接是否稳定
2. 确认图片 ID 是否有效
3. 检查后端服务器日志

## 技术细节

### 内存管理

- **PSRAM 缓冲区**：2808 字节（212x104 位图，行对齐格式）
- **优先使用 PSRAM**：8MB 外部内存
- **单缓冲策略**：只保留一张图片的位图数据

### 网络配置

- **WiFi 连接超时**：10 秒
- **HTTP 请求超时**：5 秒（认证、列表）/ 10 秒（下载）
- **认证方式**：JWT Bearer Token

### 显示策略

- **首次显示**：不使用闪烁刷新（快速显示）
- **切换图片**：使用闪烁刷新（清除残影）
- **分辨率**：212x104 像素，1-bit 黑白

## 后端 API 接口

### 认证接口

```http
POST /api/v1/auth/login
Content-Type: application/json

{
  "device_id": "ABC123",
  "password": "123456"
}
```

**响应**：
```json
{
  "access_token": "eyJhbGciOiJIUzI1Ni...",
  "token_type": "bearer"
}
```

### 获取图片列表

```http
GET /api/v1/images/
Authorization: Bearer <access_token>
```

**响应**：
```json
[
  {
    "id": 12,
    "url": "/assets/custom_images/1/abc123.png",
    "display_order": 1
  },
  {
    "id": 10,
    "url": "/assets/custom_images/1/def456.png",
    "display_order": 2
  }
]
```

### 下载位图数据

```http
GET /api/v1/images/{id}/bitmap
Authorization: Bearer <access_token>
```

**响应**：原始位图数据（2808 字节，行对齐格式）

## 安全警告

⚠️ **当前版本仅用于开发调试！**

### 已知安全问题

1. **硬编码凭证**：WiFi 密码和设备凭证直接写在代码中
2. **HTTP 传输**：未使用 HTTPS 加密
3. **无证书验证**：未验证服务器证书

### 生产环境建议

1. **使用 NVS 存储凭证**：
   ```cpp
   #include <Preferences.h>
   Preferences prefs;
   prefs.begin("config", false);
   prefs.putString("wifi_ssid", "YourSSID");
   prefs.putString("wifi_pass", "YourPassword");
   ```

2. **实现配网功能**：
   - SmartConfig（微信配网）
   - Web 配网（AP 模式）
   - 蓝牙配网

3. **使用 HTTPS**：
   ```cpp
   #include <WiFiClientSecure.h>
   WiFiClientSecure client;
   client.setCACert(root_ca);
   ```

## 故障排查

### 编译错误

**错误**：`ArduinoJson.h: No such file or directory`

**解决**：运行 `pio lib install` 安装依赖

---

**错误**：`PSRAM not found`

**解决**：检查 `platformio.ini` 是否包含 `-DBOARD_HAS_PSRAM`

### 运行时错误

**错误**：`FATAL: Failed to allocate bitmap buffer!`

**解决**：
1. 检查 PSRAM 是否正常工作
2. 尝试减少其他内存占用

---

**错误**：`Content-Length mismatch`

**解决**：
1. 检查后端返回的位图数据大小是否为 2808 字节（行对齐格式）
2. 确认图片分辨率为 212x104

## 后续优化方向

- [ ] 实现 SmartConfig 配网功能
- [ ] 使用 NVS 存储凭证
- [ ] 添加 HTTPS 支持
- [ ] 实现图片缓存（预加载下一张）
- [ ] 添加 OTA 固件更新
- [ ] 实现低功耗模式（深度睡眠）
- [ ] 添加看门狗定时器
- [ ] 实现网络自动重连

## 参考资料

- [后端 API 文档](../infinity-tag-backend/doc/教程/API接口调用规范.md)
- [ESP32 Arduino 文档](https://docs.espressif.com/projects/arduino-esp32/)
- [GxEPD2 库文档](https://github.com/ZinggJM/GxEPD2)
- [ArduinoJson 文档](https://arduinojson.org/)
