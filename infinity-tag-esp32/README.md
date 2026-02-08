# Infinity Tag ESP32 - 事件驱动墨水屏系统

基于 ESP32-S3 和墨水屏的智能显示设备，采用事件驱动架构，支持卡片系统、WiFi 配置、OTA 更新和 Lua 脚本扩展。

## 🎯 功能特性

### 核心功能
- ✅ **事件驱动架构**：基于 EventQueue 的异步事件处理
- ✅ **卡片管理系统**：可扩展的卡片框架，支持多种显示模式
- ✅ **智能内存管理**：使用 C++ 智能指针，防止内存泄漏
- ✅ **状态栏系统**：实时显示 WiFi、电量、时间等状态信息

### 网络功能
- ✅ **WiFi 配置门户**：首次启动自动创建配置热点
- ✅ **后端认证**：JWT Token 认证，支持设备激活
- ✅ **OTA 更新**：支持远程固件更新
- ✅ **图片同步**：从后端服务器获取并显示图片

### 显示功能
- ✅ **多级刷新模式**：局部刷新、闪烁刷新、全屏刷新、深度刷新
- ✅ **中文字体支持**：内置汇文仿宋字体
- ✅ **位图显示**：212x104 分辨率，1-bit 黑白显示
- ✅ **自动残影消除**：每 5 次局部刷新自动触发闪烁刷新

### 交互功能
- ✅ **旋转编码器**：卡片切换、菜单导航
- ✅ **按键控制**：
  - 单击：确认/选择
  - 长按（2秒）：进入设置
  - 三击：特殊功能触发
- ✅ **振动开关**：快速触发功能

### 扩展功能
- ✅ **Lua 脚本引擎**：支持自定义卡片逻辑
- ✅ **配置管理**：NVS 持久化存储
- ✅ **日志系统**：可控的调试输出

## 🔧 硬件要求

| 组件 | 规格 | 说明 |
|------|------|------|
| **开发板** | ESP32-S3 Gen4 R8N16 | 8MB PSRAM + 16MB Flash |
| **墨水屏** | GxEPD2_213_B72 | 212x104 分辨率，2.13 英寸 |
| **旋转编码器** | EC11 | 带按键 |
| **振动开关** | SW-18010P | 倾斜触发 |
| **电源** | 3.7V 锂电池 | 推荐 500mAh 以上 |

### 引脚配置

详见 `include/PinConfig.h`：

```cpp
// 墨水屏 SPI
#define PIN_CS      10
#define PIN_DC      11
#define PIN_RST     12
#define PIN_BUSY    13

// 旋转编码器
#define PIN_ENC_A   14
#define PIN_ENC_B   21
#define PIN_ENC_BTN 47

// 振动开关
#define PIN_SW_KEY  48

// 电源控制
#define PIN_PWR_IO  15
```

## 🚀 快速开始

### 1. 环境准备

```bash
# 安装 PlatformIO
pip install platformio

# 克隆项目
cd infinity-tag-esp32

# 安装依赖
pio lib install
```

### 2. 首次配置

**方法 A：WiFi 配置门户（推荐）**

1. 首次启动时，设备会创建热点 `InfinityTag-XXXXXX`
2. 连接热点，浏览器自动打开配置页面
3. 输入 WiFi 信息和后端服务器地址
4. 保存后自动重启并连接

**方法 B：硬编码配置（开发调试）**

编辑 `src/main.cpp`：

```cpp
#define WIFI_SSID "YourWiFiName"
#define WIFI_PASSWORD "YourPassword"
#define API_BASE_URL "http://192.168.1.100:8001"
#define DEVICE_ID "ABC123"
#define DEVICE_PASSWORD "123456"
```

### 3. 编译上传

```bash
# 编译
pio run

# 上传固件
pio run -t upload

# 查看串口输出
pio device monitor -b 115200
```

### 4. 验证运行

启动后应看到：

```
--- INFINITY TAG V2 (Event-Driven) ---
Firmware: v1.0.0.9
[OK] Hardware initialized
[OK] Core initialized
[OK] Cards loaded
[OK] Network connected
[OK] Authentication successful
--- SYSTEM READY ---
```

## 📁 项目结构

```
infinity-tag-esp32/
├── include/
│   ├── Cards/                    # 卡片系统
│   │   ├── LuaCard.h            # Lua 脚本卡片
│   │   ├── SettingsCard.h       # 设置卡片
│   │   └── WiFiConfigCard.h     # WiFi 配置卡片
│   ├── Core/                     # 核心系统
│   │   ├── Card.h               # 卡片基类
│   │   ├── CardManager.h        # 卡片管理器
│   │   ├── ConfigManager.h      # 配置管理器
│   │   ├── Event.h              # 事件定义
│   │   ├── EventQueue.h         # 事件队列
│   │   └── StatusBar.h          # 状态栏
│   ├── Driver/                   # 硬件驱动
│   │   └── EPD_Driver.h         # 墨水屏驱动
│   ├── Fonts/                    # 字体资源
│   │   └── HuiwenFangsong.h     # 汇文仿宋字体
│   ├── Input/                    # 输入管理
│   │   └── InputManager.h       # 输入管理器
│   ├── Lua/                      # Lua 引擎
│   │   ├── LuaBindings.h        # Lua 绑定
│   │   └── LuaEngine.h          # Lua 引擎
│   ├── Network/                  # 网络功能
│   │   ├── ImageFetcher.h       # 图片获取器
│   │   ├── NetworkManager.h     # 网络管理器
│   │   ├── OTAManager.h         # OTA 更新管理器
│   │   └── WiFiProvisioning.h   # WiFi 配置管理器
│   ├── Utils/                    # 工具类
│   │   ├── ChineseFont.h        # 中文字体工具
│   │   └── Logger.h             # 日志系统
│   ├── PinConfig.h              # 引脚配置
│   └── Version.h                # 版本信息
├── src/                          # 源代码实现
│   ├── Cards/
│   ├── Core/
│   ├── Driver/
│   ├── Input/
│   ├── Lua/
│   ├── Network/
│   └── main.cpp                 # 主程序入口
├── docs/                         # 文档目录
│   ├── README.md                # 架构文档
│   ├── DEBUG_GUIDE.md           # 调试指南
│   ├── NETWORK_SETUP.md         # 网络配置指南
│   ├── OTA_UPDATE_GUIDE.md      # OTA 更新指南
│   └── REFACTOR_PROGRESS.md     # 重构进度
├── platformio.ini               # PlatformIO 配置
└── README.md                    # 本文件
```

## 🎨 架构设计

### 事件驱动架构

```
┌─────────────────────────────────────────────────────────┐
│                     Main Loop                            │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │ InputManager │→ │  EventQueue  │→ │ CardManager  │  │
│  └──────────────┘  └──────────────┘  └──────────────┘  │
│         ↓                  ↓                  ↓          │
│    检测输入            事件分发            处理事件       │
└─────────────────────────────────────────────────────────┘
```

### 卡片系统

```
Card (基类)
├── LuaCard          # Lua 脚本卡片
├── SettingsCard     # 设置卡片
│   ├── WiFi 配置
│   ├── OTA 更新
│   └── 系统信息
└── WiFiConfigCard   # WiFi 配置卡片
```

### 刷新模式

| 模式 | 刷新区域 | 闪烁 | 速度 | 用途 |
|------|---------|------|------|------|
| **refreshPartial** | 局部 | 无 | 极快 | 数字跳变、菜单选择 |
| **refreshFlicker** | 局部 | 1次 | 快 | 消除残影 |
| **refreshFull** | 全屏 | 2次 | 中 | 切换卡片、场景切换 |
| **refreshDeep** | 全屏 | 3次 | 慢 | 开机初始化、休眠前 |

## 🎮 操作指南

### 按键操作

| 操作 | 功能 |
|------|------|
| **单击** | 确认/选择当前项 |
| **长按（2秒）** | 进入设置卡片 |
| **三击** | 触发特殊功能（可自定义） |

### 旋转编码器

| 操作 | 功能 |
|------|------|
| **顺时针旋转** | 下一个选项/卡片 |
| **逆时针旋转** | 上一个选项/卡片 |

### 振动开关

| 操作 | 功能 |
|------|------|
| **倾斜触发** | 快速切换/刷新 |

## 🔌 后端 API

### 认证接口

```http
POST /api/v1/auth/login
Content-Type: application/json

{
  "device_code": "ABC123",
  "password": "123456"
}
```

**响应**：
```json
{
  "access_token": "eyJhbGci...",
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
    "id": 1,
    "url": "/assets/custom_images/4/xxx.png",
    "display_order": 0
  }
]
```

### 下载位图数据

```http
GET /api/v1/images/{id}/bitmap
Authorization: Bearer <access_token>
```

**响应**：2808 字节的原始位图数据
- 格式：212x104 像素，1-bit，行对齐
- 每行 27 字节（(212+7)/8）
- 总共 27 × 104 = 2808 字节
- MSB first，0=黑色，1=白色

### OTA 更新接口

```http
GET /api/v1/ota/latest?device_id=ABC123
Authorization: Bearer <access_token>
```

## 📚 依赖库

| 库名 | 版本 | 用途 |
|------|------|------|
| **GxEPD2** | ^1.6.6 | 墨水屏驱动 |
| **Adafruit GFX Library** | ^1.12.1 | 图形绘制 |
| **ArduinoJson** | ^7.0.0 | JSON 解析 |
| **WiFiManager** | latest | WiFi 配置门户 |
| **QRCode** | ^0.0.1 | 二维码生成 |
| **U8g2_for_Adafruit_GFX** | ^1.8.0 | 中文字体支持 |

## 🛠️ 开发指南

### 添加新卡片

1. 创建头文件 `include/Cards/MyCard.h`：

```cpp
#pragma once
#include "Core/Card.h"

class MyCard : public Card {
public:
    MyCard(EPD_Driver& epd) : Card(epd, "MyCard") {}

    void onEnter() override {
        // 进入卡片时的初始化
    }

    void onEvent(const Event& event) override {
        // 处理事件
        if (event.type == EVENT_BUTTON_PRESS) {
            // 处理按键
        }
    }

    void draw() override {
        // 绘制卡片内容
        _epd.refreshFull([](EPD_Class& display) {
            display.fillScreen(GxEPD_WHITE);
            display.setTextColor(GxEPD_BLACK);
            display.setCursor(10, 50);
            display.print("My Card");
        });
    }
};
```

2. 在 `main.cpp` 中注册：

```cpp
auto myCard = std::make_unique<MyCard>(epd);
cardManager->registerCard(myCard.get());
```

### 调试日志

编辑 `include/Utils/Logger.h`：

```cpp
// 启用调试日志
#define ENABLE_DEBUG_LOGGING

// 使用日志宏
LOG_DEBUG("[MyCard] Button pressed");
LOG_PRINTF("[MyCard] Value: %d\n", value);
```

生产环境注释掉 `ENABLE_DEBUG_LOGGING` 即可禁用所有调试输出。

### 编译选项

`platformio.ini` 中的关键配置：

```ini
build_flags =
    -DBOARD_HAS_PSRAM          # 启用 PSRAM
    -DARDUINO_USB_CDC_ON_BOOT=1 # USB CDC 串口
    -DCORE_DEBUG_LEVEL=3       # ESP32 日志级别
    -DENABLE_DEBUG_LOGGING     # 启用自定义日志
```

## 🔒 安全建议

### 开发环境
- ✅ 硬编码凭证（快速测试）
- ✅ HTTP 传输（局域网）
- ✅ 调试日志开启

### 生产环境
- ⚠️ 使用 WiFi 配置门户
- ⚠️ NVS 加密存储凭证
- ⚠️ HTTPS + 证书验证
- ⚠️ 关闭调试日志
- ⚠️ 启用看门狗定时器
- ⚠️ 实现固件签名验证

## 🐛 故障排查

### WiFi 连接失败

**症状**：无法连接 WiFi

**解决方案**：
1. 检查 SSID 和密码是否正确
2. 确认路由器开启 2.4GHz 频段（ESP32 不支持 5GHz）
3. 查看串口输出的错误信息
4. 尝试重置 WiFi 配置（长按进入设置）

### 认证失败 (HTTP 422)

**症状**：`Authentication failed! HTTP code: 422`

**原因**：字段名称不匹配

**解决方案**：
- 确认使用 `device_code` 而不是 `device_id`
- 检查后端 API 版本是否匹配

### 位图下载失败

**症状**：`Content-Length mismatch (expected 2808, got XXXX)`

**原因**：位图格式不匹配

**解决方案**：
1. 确认后端返回行对齐格式（2808 字节）
2. 重新上传图片（使用新版后端）
3. 检查图片尺寸是否为 212x104

### 内存不足

**症状**：`FATAL: Failed to allocate bitmap buffer!`

**解决方案**：
1. 确认 PSRAM 正常工作：`psramFound()` 返回 true
2. 检查 `platformio.ini` 包含 `-DBOARD_HAS_PSRAM`
3. 减少全局变量使用
4. 使用智能指针管理内存

### 看门狗重启

**症状**：设备频繁重启，串口显示 `Task watchdog got triggered`

**解决方案**：
1. 检查是否有死循环或长时间阻塞
2. 在 `loop()` 中定期调用 `esp_task_wdt_reset()`
3. 增加看门狗超时时间（当前 30 秒）

## 📊 性能指标

| 指标 | 数值 | 说明 |
|------|------|------|
| **启动时间** | ~5 秒 | 从上电到系统就绪 |
| **WiFi 连接** | ~3 秒 | 连接到已知网络 |
| **认证时间** | ~1 秒 | 后端 JWT 认证 |
| **图片下载** | ~2 秒 | 2808 字节位图 |
| **局部刷新** | ~300ms | refreshPartial |
| **全屏刷新** | ~2 秒 | refreshFull |
| **深度刷新** | ~4 秒 | refreshDeep |
| **内存使用** | ~150KB | 运行时 RAM |
| **PSRAM 使用** | ~3KB | 位图缓冲区 |

## 📝 版本历史

### v1.0.0.9 (2026-02-08) - 架构重构

**重大变更**：
- ✅ 重构为事件驱动架构
- ✅ 引入卡片管理系统
- ✅ 使用智能指针管理内存
- ✅ 添加状态栏系统
- ✅ 支持长按和三击检测
- ✅ 添加日志系统（可控调试输出）
- ✅ 优化看门狗策略（30秒超时 + 定期喂狗）

**功能新增**：
- ✅ WiFi 配置门户
- ✅ OTA 更新支持
- ✅ Lua 脚本引擎
- ✅ 中文字体支持
- ✅ 配置持久化（NVS）

**Bug 修复**：
- ✅ 修复内存泄漏问题
- ✅ 修复位图格式不匹配（2808 字节行对齐）
- ✅ 修复认证字段错误（device_code）
- ✅ 修复端口配置（8001）

### v1.0.0.1 (2026-02-07) - 初始版本

- ✅ 基础 WiFi 连接
- ✅ 后端认证
- ✅ 图片下载显示
- ✅ 按键和编码器控制

## 🗺️ 后续计划

### 短期（v1.2.0）
- [ ] 图片缓存（预加载下一张）
- [ ] 低功耗模式（深度睡眠）
- [ ] 电量监控和显示
- [ ] 网络自动重连
- [ ] 固件签名验证

### 中期（v1.3.0）
- [ ] 蓝牙配网支持
- [ ] 多语言支持
- [ ] 主题系统
- [ ] 动画效果
- [ ] 触摸屏支持

### 长期（v2.0.0）
- [ ] 完整的 Lua 卡片生态
- [ ] 云端卡片商店
- [ ] AI 内容生成
- [ ] 多设备同步
- [ ] 开放 API 平台

## 📖 参考资料

### 项目文档
- [架构设计文档](./docs/README.md)
- [调试指南](./docs/DEBUG_GUIDE.md)
- [网络配置指南](./docs/NETWORK_SETUP.md)
- [OTA 更新指南](./docs/OTA_UPDATE_GUIDE.md)
- [重构进度](./docs/REFACTOR_PROGRESS.md)

### 后端文档
- [后端 API 文档](../infinity-tag-backend/doc/教程/API接口调用规范.md)
- [端口修改指南](../PORT_CHANGE_GUIDE.md)

### 外部资源
- [ESP32 Arduino 文档](https://docs.espressif.com/projects/arduino-esp32/)
- [GxEPD2 库文档](https://github.com/ZinggJM/GxEPD2)
- [Adafruit GFX 教程](https://learn.adafruit.com/adafruit-gfx-graphics-library)
- [PlatformIO 文档](https://docs.platformio.org/)

## 🤝 贡献指南

欢迎提交 Issue 和 Pull Request！

### 代码规范
- 使用 4 空格缩进
- 类名使用 PascalCase
- 函数名使用 camelCase
- 常量使用 UPPER_SNAKE_CASE
- 添加必要的注释

### 提交规范
```
feat: 添加新功能
fix: 修复 Bug
docs: 更新文档
refactor: 重构代码
perf: 性能优化
test: 添加测试
chore: 构建/工具链更新
```

## 📄 许可证

MIT License

## 👥 作者

Infinity Tag Team

---

**最后更新**：2026-02-08
**固件版本**：v1.0.0.9
**文档版本**：v2.0
