# Infinity Tag ESP32 - 架构设计文档

本文档详细描述了 Infinity Tag ESP32 固件的架构设计、模块划分和实现细节。

## 📐 架构概览

### 分层架构

```
┌─────────────────────────────────────────────────────────────┐
│                      Application Layer                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  LuaCard     │  │ SettingsCard │  │WiFiConfigCard│      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
├─────────────────────────────────────────────────────────────┤
│                      Core Layer                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ CardManager  │  │  EventQueue  │  │  StatusBar   │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│  ┌──────────────┐  ┌──────────────┐                        │
│  │ConfigManager │  │  LuaEngine   │                        │
│  └──────────────┘  └──────────────┘                        │
├─────────────────────────────────────────────────────────────┤
│                      Service Layer                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ImageFetcher  │  │ OTAManager   │  │WiFiProvision │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│  ┌──────────────┐                                           │
│  │NetworkManager│                                           │
│  └──────────────┘                                           │
├─────────────────────────────────────────────────────────────┤
│                      Driver Layer                            │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  EPD_Driver  │  │InputManager  │  │ ChineseFont  │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
├─────────────────────────────────────────────────────────────┤
│                      Hardware Layer                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  GxEPD2      │  │   WiFi       │  │    NVS       │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────────────────────────────────────────────┘
```

## 🎯 核心模块

### 1. 事件系统 (Core/Event)

#### Event.h - 事件定义

```cpp
enum EventType {
    EVENT_NONE = 0,
    EVENT_BUTTON_PRESS,          // 按键按下
    EVENT_BUTTON_RELEASE,        // 按键松开
    EVENT_BUTTON_LONG_PRESS,     // 长按（2秒）
    EVENT_BUTTON_TRIPLE_CLICK,   // 三击
    EVENT_ENCODER_ROTATE,        // 编码器旋转
    EVENT_VIBRATION,             // 振动触发
    EVENT_NETWORK_CONNECTED,     // 网络连接
    EVENT_NETWORK_DISCONNECTED,  // 网络断开
    EVENT_OTA_AVAILABLE,         // OTA 更新可用
    EVENT_CARD_SWITCH,           // 卡片切换
};

struct Event {
    EventType type;
    int value;  // 可选参数（如编码器增量）
};
```

#### EventQueue.h - 事件队列

**职责**：
- 管理事件队列（FIFO）
- 线程安全的事件推送和弹出
- 防止队列溢出

**关键方法**：
```cpp
void push(const Event& event);  // 推送事件
bool pop(Event& event);         // 弹出事件
bool isEmpty();                 // 检查队列是否为空
void clear();                   // 清空队列
```

**实现细节**：
- 使用固定大小的循环缓冲区（32 个事件）
- 使用互斥锁保证线程安全
- 队列满时丢弃最旧的事件

### 2. 卡片系统 (Core/Card)

#### Card.h - 卡片基类

```cpp
class Card {
public:
    virtual void onEnter() = 0;           // 进入卡片
    virtual void onExit() = 0;            // 退出卡片
    virtual void onEvent(const Event&) = 0; // 处理事件
    virtual void draw() = 0;              // 绘制卡片
    virtual const char* getName() = 0;    // 获取卡片名称

protected:
    EPD_Driver& _epd;
    String _name;
};
```

#### CardManager.h - 卡片管理器

**职责**：
- 管理所有注册的卡片
- 处理卡片切换
- 分发事件到当前卡片

**关键方法**：
```cpp
void registerCard(Card* card);     // 注册卡片
void switchCard(int index);        // 切换到指定卡片
void nextCard();                   // 下一张卡片
void prevCard();                   // 上一张卡片
void handleEvent(const Event& e);  // 处理事件
int getCardCount();                // 获取卡片数量
Card* getCurrentCard();            // 获取当前卡片
```

**实现细节**：
- 使用 `std::vector<Card*>` 存储卡片指针
- 卡片切换时调用 `onExit()` 和 `onEnter()`
- 使用 `refreshFull()` 刷新屏幕

### 3. 配置管理 (Core/ConfigManager)

**职责**：
- 管理设备配置（WiFi、API、设备信息）
- NVS 持久化存储
- 配置的读取和写入

**配置项**：
```cpp
struct Config {
    String wifi_ssid;
    String wifi_password;
    String api_base_url;
    String device_id;
    String device_password;
    bool first_boot;
};
```

**关键方法**：
```cpp
bool load();                    // 从 NVS 加载配置
bool save();                    // 保存配置到 NVS
void reset();                   // 重置为默认值
String get(const char* key);    // 获取配置项
void set(const char* key, const String& value); // 设置配置项
```

### 4. 状态栏 (Core/StatusBar)

**职责**：
- 显示系统状态信息
- WiFi 信号强度
- 电量显示
- 时间显示

**显示内容**：
```
┌─────────────────────────────────────┐
│ [WiFi] [Battery] [Time]             │ ← 状态栏
├─────────────────────────────────────┤
│                                     │
│         卡片内容区域                 │
│                                     │
└─────────────────────────────────────┘
```

**关键方法**：
```cpp
void draw();                    // 绘制状态栏
void update();                  // 更新状态信息
void setWiFiStatus(bool connected, int rssi);
void setBatteryLevel(int level);
void setTime(const char* time);
```

## 🔌 驱动层

### 1. EPD_Driver - 墨水屏驱动

**职责**：
- 封装 GxEPD2 库
- 提供统一的刷新接口
- 管理刷新策略

**刷新模式**：

| 模式 | 方法 | 闪烁次数 | 速度 | 用途 |
|------|------|---------|------|------|
| 局部刷新 | `refreshPartial()` | 0 | 300ms | 数字跳变 |
| 闪烁刷新 | `refreshFlicker()` | 1 | 1s | 消除残影 |
| 全屏刷新 | `refreshFull()` | 2 | 2s | 切换卡片 |
| 深度刷新 | `refreshDeep()` | 3 | 4s | 开机/休眠 |

**自动残影消除**：
- 每 5 次局部刷新自动触发一次闪烁刷新
- 计数器可通过 `resetPartialCounter()` 重置

**回调函数模式**：
```cpp
epd.refreshFull([](EPD_Class& display) {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(10, 50);
    display.print("Hello World");
});
```

**硬件偏移**：
- 物理屏幕：250x122
- 可见区域：212x104
- Y 轴偏移：18 像素（`OFFSET_Y`）
- 所有绘制必须加上 `OFFSET_Y`

### 2. InputManager - 输入管理

**职责**：
- 管理旋转编码器、按键、振动开关
- 检测长按和三击
- 生成输入事件

**输入检测**：

```cpp
void update(EventQueue& eventQueue) {
    // 1. 检测编码器旋转
    int8_t delta = getEncoderDelta();
    if (delta != 0) {
        eventQueue.push(Event(EVENT_ENCODER_ROTATE, delta));
    }

    // 2. 检测按键
    if (isButtonPressed()) {
        eventQueue.push(Event(EVENT_BUTTON_PRESS));
        _checkTripleClick(eventQueue);
    }

    // 3. 检测长按
    if (_buttonDown && !_longPressTriggered) {
        _checkLongPress(eventQueue);
    }

    // 4. 检测振动
    if (isVibrationTriggered()) {
        eventQueue.push(Event(EVENT_VIBRATION));
    }
}
```

**长按检测**：
- 阈值：2000ms
- 触发后设置标志位防止重复触发

**三击检测**：
- 时间窗口：500ms
- 计数器在窗口外重置
- 使用 `>=` 防止计数器溢出

### 3. ChineseFont - 中文字体

**职责**：
- 加载汇文仿宋字体
- 提供中文字符绘制接口
- 支持多种字号

**使用方式**：
```cpp
ChineseFont font;
font.drawString(display, "你好世界", 10, 50, 16);
```

## 🌐 网络层

### 1. NetworkManager - 网络管理

**职责**：
- WiFi 连接管理
- 自动重连
- 信号强度监控

**连接流程**：
```
1. 读取配置（SSID + Password）
2. 尝试连接（10秒超时）
3. 连接成功 → 发送 EVENT_NETWORK_CONNECTED
4. 连接失败 → 启动配置门户
```

### 2. WiFiProvisioning - WiFi 配置

**职责**：
- 创建配置热点
- 提供 Web 配置界面
- 保存配置到 NVS

**配置流程**：
```
1. 创建热点：InfinityTag-XXXXXX
2. 用户连接热点
3. 自动打开配置页面（Captive Portal）
4. 输入 WiFi 信息和后端地址
5. 保存配置并重启
```

### 3. ImageFetcher - 图片获取

**职责**：
- 后端认证（JWT）
- 获取图片列表
- 下载位图数据

**API 调用流程**：
```
1. authenticate() → 获取 access_token
2. fetchImageList() → 获取图片列表
3. downloadBitmap() → 下载位图数据（2808 字节）
```

**认证修复**：
- 使用 `device_code` 而不是 `device_id`
- 匹配后端 API v1.0

**位图格式**：
- 尺寸：212x104 像素
- 格式：1-bit，行对齐
- 大小：2808 字节（27 bytes/row × 104 rows）
- 编码：MSB first，0=黑色，1=白色

### 4. OTAManager - OTA 更新

**职责**：
- 检查固件更新
- 下载并安装固件
- 显示更新进度

**更新流程**：
```
1. 检查更新：GET /api/v1/ota/latest
2. 下载固件：GET /api/v1/ota/download/{version}
3. 验证签名（可选）
4. 写入 Flash
5. 重启设备
```

## 🎨 应用层

### 1. SettingsCard - 设置卡片

**功能菜单**：
```
┌─────────────────────────┐
│ 设置                     │
├─────────────────────────┤
│ > WiFi 配置              │
│   OTA 更新               │
│   系统信息               │
│   重启设备               │
└─────────────────────────┘
```

**交互方式**：
- 编码器：上下选择
- 按键：确认选择
- 长按：返回上级

### 2. WiFiConfigCard - WiFi 配置卡片

**功能**：
- 扫描 WiFi 网络
- 选择并连接
- 显示连接状态

### 3. LuaCard - Lua 脚本卡片

**职责**：
- 加载 Lua 脚本
- 执行 Lua 逻辑
- 提供 API 绑定

**Lua API**：
```lua
-- 显示文本
display.print(x, y, text)

-- 绘制图形
display.drawRect(x, y, w, h)
display.fillRect(x, y, w, h)

-- 刷新屏幕
display.refresh()

-- 网络请求
http.get(url)
http.post(url, data)

-- 配置读写
config.get(key)
config.set(key, value)
```

## 🔧 工具类

### 1. Logger - 日志系统

**日志级别**：
```cpp
#ifdef ENABLE_DEBUG_LOGGING
#define LOG_DEBUG(msg) Serial.println(msg)
#define LOG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define LOG_DEBUG(msg)
#define LOG_PRINTF(...)
#endif
```

**使用方式**：
```cpp
LOG_DEBUG("[CardManager] Switching to card 1");
LOG_PRINTF("[Network] RSSI: %d dBm\n", rssi);
```

**生产环境**：
- 注释掉 `ENABLE_DEBUG_LOGGING`
- 所有日志宏变为空操作
- 零性能开销

## 🔄 主循环流程

```cpp
void loop() {
    // 1. 更新输入管理器（生成事件）
    input.update(eventQueue);

    // 2. 处理事件队列
    Event event;
    while (eventQueue.pop(event)) {
        // 特殊事件处理
        if (event.type == EVENT_BUTTON_LONG_PRESS) {
            cardManager->switchCard(SETTINGS_CARD_INDEX);
        } else {
            // 分发到当前卡片
            cardManager->handleEvent(event);
        }
    }

    // 3. 更新状态栏
    statusBar.update();

    // 4. 喂狗
    esp_task_wdt_reset();

    delay(10);
}
```

## 💾 内存管理

### 智能指针使用

```cpp
// 全局对象使用 unique_ptr
std::unique_ptr<CardManager> cardManager;
std::unique_ptr<WiFiProvisioning> wifiProvisioning;
std::unique_ptr<OTAManager> otaManager;

// 初始化
cardManager = std::unique_ptr<CardManager>(new CardManager(epd, statusBar));

// 自动释放，无需手动 delete
```

**优势**：
- ✅ 自动内存管理
- ✅ 防止内存泄漏
- ✅ 异常安全
- ✅ 明确所有权

### PSRAM 使用

```cpp
// 位图缓冲区使用 PSRAM
if (psramFound()) {
    g_bitmapBuffer = (uint8_t*)ps_malloc(2808);
} else {
    g_bitmapBuffer = (uint8_t*)malloc(2808);
}

// 使用前检查
if (g_bitmapBuffer == nullptr) {
    Serial.println("FATAL: Failed to allocate bitmap buffer!");
    while(1);
}

// 释放旧内存（防止泄漏）
if (g_bitmapBuffer != nullptr) {
    free(g_bitmapBuffer);
    g_bitmapBuffer = nullptr;
}
```

### 内存优化建议

1. **避免大型全局变量**
   - 使用动态分配
   - 使用 PSRAM 存储大数据

2. **字符串优化**
   - 使用 `F()` 宏存储常量字符串到 Flash
   - 避免 String 拼接，使用 `snprintf()`

3. **栈空间管理**
   - 避免大型局部数组
   - 递归深度控制

## 🔐 安全考虑

### 1. 凭证存储

**开发环境**：
```cpp
#define WIFI_SSID "MyWiFi"
#define WIFI_PASSWORD "password"
```

**生产环境**：
```cpp
// 使用 NVS 加密存储
nvs_set_str(handle, "wifi_ssid", ssid);
nvs_set_str(handle, "wifi_pass", password);
```

### 2. HTTPS 支持

```cpp
// 添加根证书
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"...\n" \
"-----END CERTIFICATE-----\n";

// 使用 WiFiClientSecure
WiFiClientSecure client;
client.setCACert(root_ca);
```

### 3. 固件签名验证

```cpp
// OTA 更新前验证签名
bool verifyFirmware(const uint8_t* firmware, size_t size) {
    // 1. 提取签名
    // 2. 计算哈希
    // 3. 验证签名
    return true;
}
```

## 📊 性能优化

### 1. 刷新策略

- 优先使用局部刷新（最快）
- 每 5 次局部刷新触发一次闪烁刷新
- 卡片切换使用全屏刷新
- 开机/休眠使用深度刷新

### 2. 网络优化

- 使用 HTTP Keep-Alive
- 图片预加载（下一张）
- 断线自动重连

### 3. 功耗优化

- 空闲时进入 Light Sleep
- 墨水屏休眠（`hibernate()`）
- WiFi 省电模式

## 🔒 看门狗策略

### 配置

```cpp
// 配置看门狗（30秒超时）
esp_task_wdt_deinit();
esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 30000,                   // 30秒超时
    .idle_core_mask = (1 << 0) | (1 << 1), // 两个核心都监控
    .trigger_panic = true                  // 超时时重启
};
esp_task_wdt_init(&wdt_config);
esp_task_wdt_add(NULL);
```

### 喂狗

```cpp
void loop() {
    // ... 处理逻辑

    // 定期喂狗
    esp_task_wdt_reset();

    delay(10);
}
```

**优势**：
- ✅ 保留看门狗保护
- ✅ 死锁时自动重启
- ✅ 30秒超时足够长
- ✅ 定期喂狗防止误触发

## 🧪 测试建议

### 1. 单元测试

```cpp
// 测试事件队列
void test_event_queue() {
    EventQueue queue;
    Event e1(EVENT_BUTTON_PRESS);
    queue.push(e1);

    Event e2;
    assert(queue.pop(e2));
    assert(e2.type == EVENT_BUTTON_PRESS);
}
```

### 2. 集成测试

- WiFi 连接测试
- 后端认证测试
- 图片下载测试
- OTA 更新测试

### 3. 压力测试

- 长时间运行测试（24小时+）
- 内存泄漏检测
- 看门狗触发测试

## 📈 性能指标

| 指标 | 目标 | 当前 | 说明 |
|------|------|------|------|
| **启动时间** | < 10s | ~5s | 从上电到系统就绪 |
| **WiFi 连接** | < 5s | ~3s | 连接到已知网络 |
| **认证时间** | < 2s | ~1s | 后端 JWT 认证 |
| **图片下载** | < 3s | ~2s | 2808 字节位图 |
| **局部刷新** | < 500ms | ~300ms | refreshPartial |
| **全屏刷新** | < 3s | ~2s | refreshFull |
| **深度刷新** | < 5s | ~4s | refreshDeep |
| **内存使用** | < 200KB | ~150KB | 运行时 RAM |
| **PSRAM 使用** | < 10KB | ~3KB | 位图缓冲区 |

## 🔄 数据流

### 图片显示流程

```
┌─────────┐     ┌──────────┐     ┌─────────┐     ┌─────────┐
│  用户   │────>│ 按键按下 │────>│事件队列 │────>│卡片管理 │
└─────────┘     └──────────┘     └─────────┘     └─────────┘
                                                        │
                                                        ▼
┌─────────┐     ┌──────────┐     ┌─────────┐     ┌─────────┐
│墨水屏   │<────│位图数据  │<────│后端API  │<────│图片获取 │
└─────────┘     └──────────┘     └─────────┘     └─────────┘
```

### 配置流程

```
┌─────────┐     ┌──────────┐     ┌─────────┐     ┌─────────┐
│  用户   │────>│长按按键  │────>│设置卡片 │────>│WiFi配置 │
└─────────┘     └──────────┘     └─────────┘     └─────────┘
                                                        │
                                                        ▼
┌─────────┐     ┌──────────┐     ┌─────────┐     ┌─────────┐
│ 重启    │<────│保存配置  │<────│  NVS    │<────│配置管理 │
└─────────┘     └──────────┘     └─────────┘     └─────────┘
```

## 📚 参考资料

### 官方文档
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [GxEPD2 库](https://github.com/ZinggJM/GxEPD2)
- [ArduinoJson](https://arduinojson.org/)
- [WiFiManager](https://github.com/tzapu/WiFiManager)

### 设计模式
- 事件驱动架构
- 观察者模式（事件系统）
- 策略模式（刷新策略）
- 工厂模式（卡片创建）

### 相关文档
- [主 README](../README.md)
- [重构进度](./REFACTOR_PROGRESS.md)
- [网络配置指南](./NETWORK_SETUP.md)
- [调试指南](./DEBUG_GUIDE.md)

---

**文档版本**：v2.0
**最后更新**：2026-02-08
**对应固件版本**：v1.0.0.9
**维护者**：Infinity Tag Team
