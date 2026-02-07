# WiFi配网问题修复报告

## 🐛 问题描述

### 问题1：WiFi启动了两次
从日志可以看到WiFiConfigCard的`onEnter()`被调用了两次：
- 第一次：在`registerCard()`时自动调用（因为是第一张卡片）
- 第二次：在`setup()`中显式调用`setCurrentCard(0)`

### 问题2：扫码连上后不会跳出强制门户
Captive Portal需要DNS服务器和Web服务器在主循环中持续处理请求，但原代码没有调用它们的处理方法。

---

## ✅ 修复方案

### 修复1：移除自动激活逻辑

**文件**：`src/Core/CardManager.cpp`

**修改**：在`registerCard()`方法中移除自动调用`setCurrentCard(0)`的逻辑

```cpp
void CardManager::registerCard(Card* card) {
    if (card == nullptr) {
        Serial.println("[CardManager] ERROR: Cannot register null card");
        return;
    }

    _cards.push_back(card);
    Serial.printf("[CardManager] Registered card: %s (%s)\n",
                  card->getName().c_str(), card->getCategory().c_str());

    // 注意：不自动设置当前卡片，由调用者决定何时激活
}
```

**原因**：让调用者明确控制何时激活卡片，避免重复激活。

---

### 修复2：添加update()方法处理Captive Portal

**文件1**：`include/Cards/WiFiConfigCard.h`

**添加**：
```cpp
/**
 * @brief 更新方法（处理DNS和Web请求）
 *
 * 必须在主循环中调用以处理Captive Portal
 */
void update();
```

**文件2**：`src/Cards/WiFiConfigCard.cpp`

**添加**：
```cpp
void WiFiConfigCard::update() {
    if (!_isAPStarted) {
        return;
    }

    // 处理DNS请求（Captive Portal）
    if (_dnsServer) {
        _dnsServer->processNextRequest();
    }

    // 处理Web请求
    if (_webServer) {
        _webServer->handleClient();
    }
}
```

**原因**：
- DNS服务器需要调用`processNextRequest()`来重定向所有域名到AP的IP
- Web服务器需要调用`handleClient()`来处理HTTP请求
- 这两个方法必须在主循环中持续调用才能实现Captive Portal功能

---

### 修复3：在主循环中调用update()

**文件**：`src/main.cpp`

**修改**：在`loop()`函数开头添加：
```cpp
// 1. 如果在WiFi配网模式，处理DNS和Web请求（Captive Portal）
if (wifiConfigCard && cardManager->getCurrentCardIndex() == 0) {
    wifiConfigCard->update();
}
```

**原因**：确保在WiFi配网模式下，DNS和Web服务器能够持续处理请求。

---

## 🧪 测试步骤

### 1. 编译并上传固件
```bash
cd infinity-tag-esp32
pio run -t upload
```

### 2. 观察启动日志
应该只看到一次WiFi启动：
```
[WiFiConfigCard] Entering WiFi config mode
[WiFiConfigCard] Starting AP mode...
[WiFiConfigCard] APted: InfinityTag-xxxx
[WiFiConfigCard] AP password: xxxxxxxx
[WiFiConfigCard] AP IP: 192.168.4.1
[WiFiConfigCard] Web server started
```

### 3. 测试Captive Portal

#### 方法1：扫描二维码
1. 使用手机扫描屏幕上的二维码
2. 手机应该自动连接到AP
3. **关键**：连接后应该自动弹出配置页面（Captive Portal）

#### 方法2：手动连接
1. 手机WiFi设置中找到"InfinityTag-xxxx"
2. 输入屏幕上显示的密码连接
3. 连接后打开浏览器访问任意网址（如baidu.com）
4. 应该自动跳转到配置页面（192.168.4.1）

### 4. 配置WiFi
1. 在配置页面输入家里的WiFi名称和密码
2. 点击"Save & Connect"
3. 设备应该保存配置并重启
4. 重启后自动连接到家里的WiFi

---

## 📊 预期日志输出

### 正常启动（首次启动）
```
========================================
  INFINITY TAG V2 - Lua Card Engine
========================================
[CRITICAL] PWR_IO set to HIGH - Power hold enabled
Firmware: v1.0.0.1
[OK] Hardware initialized
[LittleFS] Initializing...
[LittleFS] Mounted successfully
[LittleFS] Total: 3538944 bytes, Used: 8192 bytes
[OK] LittleFS initialized
[ConfigManager] Initialized
[OK] Config loaded
[StatusBar] Initialized
[CardManager] Initialized
[OK] Core initialized
[Cards] Initializing cards...
[CardManager] Registered card: WiFi配网 (系统)
[Cards] Registered 1 cards
[OK] Cards loaded
[Setup] First boot or no WiFi config - entering config mode
[WiFiConfigCard] Entering WiFi config mode
[WiFiConfigCard] Starting AP mode...
[WiFiConfigCard] APted: InfinityTag-f3c
[WiFiConfigCard] AP password: 12345678
[WiFiConfigCard] AP IP: 192.168.4.1
[WiFiConfigCard] Web server started
[WiFiConfigCard] Rendering config UI
[WiFiConfigCard] Config UI rendered
[CardManager] Switched to card: WiFi配网
========================================
  SYSTEM READY (Config Mode)
========================================
```

### 配置提交后
```
[WiFiConfigCard] Received config submission
[WiFiConfigCard] SSID: YourWiFiName
[WiFiConfigCard] Password: [hidden]
[ConfigManager] WiFi SSID set: YourWiFiName
[ConfigManager] WiFi password set
```

---

## 🔍 Captive Portal工作原理

### 1. DNS劫持
- DNS服务器监听53端口
- 将所有域名解析请求重定向到AP的IP（192.168.4.1）
- 用户访问任何网址都会被重定向到配置页面

### 2. Web服务器
- 监听80端口
- 提供配置页面HTML
- 处理配置提交（POST /config）
- 404请求重定向到根路径（Captive Portal特性）

### 3. 主循环处理
```cpp
void loop() {
    // 处理DNS请求（每次循环）
    _dnsServer->processNextRequest();

    // 处理Web请求（每次循环）
    _webServer->handleClient();
}
```

---

## ⚠️ 注意事项

### 1. iOS设备
- iOS会自动检测Captive Portal
- 连接WiFi后会自动弹出配置页面
- 如果没有弹出，打开Safari访问任意网址

### 2. Android设备
- Android也会检测Captive Portal
- 连接WiFi后通知栏会显示"需要登录"
- 点击通知即可打开配置页面

### 3. Windows/Mac
- 需要手动打开浏览器
- 访问任意HTTP网址（不要用HTTPS）
- 会自动跳转到192.168.4.1

### 4. 调试技巧
如果Captive Portal不工作：
1. 检查DNS服务器是否启动：`_dnsServer->start(53, "*", _apIP)`
2. 检查Web服务器是否启动：`_webServer->begin()`
3. 检查主循环是否调用：`update()`方法
4. 手动访问：`http://192.168.4.1`

---

## 📝 修改文件清单

1. ✅ `src/Core/CardManager.cpp` - 移除自动激活逻辑
2. ✅ `include/Cards/WiFiConfigCard.h` - 添加update()方法声明
3. ✅ `src/Cards/WiFiConfigCard.cpp` - 实现update()方法
4. ✅ `src/main.cpp` - 在loop中调用update()

---

## 🎉 修复完成

现在WiFi配网功能应该正常工作了：
- ✅ 只启动一次AP
- ✅ Captive Portal自动弹出
- ✅ 配置保存并重启

请重新编译上传固件测试！

---

生成时间：2026-02-07
版本：v1.0
