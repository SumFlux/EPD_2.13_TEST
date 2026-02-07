# Infinity Tag ESP32 - 网络图片显示系统

基于 ESP32-S3 和墨水屏的网络图片显示设备，支持从后端服务器获取并显示自定义图片。

## 功能特性

- ✅ **WiFi 连接**：自动连接到指定 WiFi 网络
- ✅ **后端认证**：JWT Token 认证
- ✅ **图片获取**：从后端服务器获取图片列表
- ✅ **位图显示**：在 212x104 墨水屏上显示 1-bit 位图
- ✅ **交互控制**：
  - 按键：获取图片列表并显示第一张
  - 旋转编码器：切换图片（带闪烁刷新）
  - 振动开关：计数器递增（保留功能）

## 硬件要求

- **开发板**：ESP32-S3 Gen4 R8N16（8MB PSRAM + 16MB Flash）
- **墨水屏**：GxEPD2_213_B72（212x104 分辨率）
- **输入设备**：
  - 旋转编码器
  - 按键开关
  - 振动开关

## 快速开始

### 1. 安装依赖

```bash
# 使用 PlatformIO
pio lib install
```

### 2. 配置参数

编辑 `src/main.cpp`，修改以下配置：

```cpp
#define WIFI_SSID "YourWiFiName"          // WiFi 名称
#define WIFI_PASSWORD "YourPassword"      // WiFi 密码
#define API_BASE_URL "http://192.168.1.100:8000"  // 后端地址
#define DEVICE_ID "ABC123"                // 设备 ID
#define DEVICE_PASSWORD "123456"          // 设备密码
```

### 3. 编译上传

```bash
pio run -t upload
pio device monitor -b 115200
```

详细配置说明请参考 [NETWORK_SETUP.md](./NETWORK_SETUP.md)

## 项目结构

```
infinity-tag-esp32/
├── include/
│   ├── Driver/
│   │   └── EPD_Driver.h          # 墨水屏驱动
│   ├── Input/
│   │   └── InputManager.h        # 输入管理器
│   ├── Network/
│   │   ├── NetworkManager.h      # WiFi 管理器
│   │   └── ImageFetcher.h        # 图片获取器
│   ├── PinConfig.h               # 引脚配置
│   └── Version.h                 # 版本信息
├── src/
│   ├── Driver/
│   │   └── EPD_Driver.cpp
│   ├── Input/
│   │   └── InputManager.cpp
│   ├── Network/
│   │   ├── NetworkManager.cpp
│   │   └── ImageFetcher.cpp
│   └── main.cpp                  # 主程序
├── platformio.ini                # PlatformIO 配置
├── NETWORK_SETUP.md              # 网络配置指南
└── README.md                     # 本文件
```

## 依赖库

- **GxEPD2** (^1.6.6) - 墨水屏驱动
- **Adafruit GFX Library** (^1.12.1) - 图形库
- **ArduinoJson** (^7.0.0) - JSON 解析

## 使用说明

### 启动流程

1. 设备上电后自动连接 WiFi（10 秒超时）
2. 认证后端服务器获取 access_token
3. 显示 "System Ready" 提示画面

### 操作方式

| 操作 | 功能 |
|------|------|
| 按键按下 | 获取图片列表并显示第一张图片 |
| 旋转编码器 | 切换图片（循环切换，带闪烁刷新） |
| 振动开关 | 计数器递增（0-99 循环） |

### 串口日志

```
--- INFINITY TAG V2 (Network)ware: v1.0.0.1
Connecting to WiFi...........
WiFi Connected!
IP: 192.168.1.123
Authenticating...
Authentication successful!
--- SYSTEM READY ---

[BTN] Fetch images
Fetched 3 images
Downloading bitmap for image 12...
Downloaded 2808 bytes
Bitmap displayed

[ENC] Switch image: 1 (delta=1)
Downloading bitmap for image 10...
Downloaded 2808 bytes
Bitmap displayed
```

## 后端 API

### 认证接口

```http
POST /api/v1/auth/login
Content-Type: application/json

{
  "device_id": "ABC123",
  "password": "123456"
}
```

### 获取图片列表

```http
GET /api/v1/images/
Authorization: Bearer <access_token>
```

### 下载位图数据

```http
GET /api/v1/images/{id}/bitmap
Authorization: Bearer <access_token>
```

返回 2808 字节的原始位图数据（212x104 像素，1-bit，行对齐格式）

## 安全警告

⚠️ **当前版本仅用于开发调试！**

- WiFi 密码和设备凭证硬编码在代码中
- 使用 HTTP 传输（未加密）
- 未验证服务器证书

**生产环境建议**：
- 使用 NVS 存储凭证
- 实现 SmartConfig 或 Web 配网
- 使用 HTTPS + 证书验证

## 故障排查

### WiFi 连接失败

- 检查 SSID 和密码是否正确
- 确认路由器开启 2.4GHz 频段（ESP32 不支持 5GHz）

### 认证失败

- 检查 DEVICE_ID 和 DEVICE_PASSWORD 是否正确
- 确认后端服务器地址和端口

### 无图片

- 登录后端管理界面上传图片
- 确认图片已分配给当前设备

### 内存错误

- 检查 PSRAM 是否正常工作
- 确认 `platformio.ini` 包含 `-DBOARD_HAS_PSRAM`

## 版本历史

### v1.0.0.1 (2026-02-07)

- ✅ 实现 WiFi 连接管理
- ✅ 实现后端认证（JWT）
- ✅ 实现图片列表获取
- ✅ 实现位图下载和显示
- ✅ 实现按键触发和编码器切换
- ✅ 添加错误处理和状态显示

## 后续计划

- [ ] SmartConfig 配网功能
- [ ] NVS 凭证存储
- [ ] HTTPS 支持
- [ ] 图片缓存（预加载）
- [ ] OTA 固件更新
- [ ] 低功耗模式（深度睡眠）
- [ ] 看门狗定时器
- [ ] 网络自动重连

## 参考资料

- [网络配置指南](./NETWORK_SETUP.md)
- [后端 API 文档](../infinity-tag-backend/doc/教程/API接口调用规范.md)
- [ESP32 Arduino 文档](https://docs.espressif.com/projects/arduino-esp32/)
- [GxEPD2 库文档](https://github.com/ZinggJM/GxEPD2)

## 许可证

MIT License

## 作者

Infinity Tag Team
