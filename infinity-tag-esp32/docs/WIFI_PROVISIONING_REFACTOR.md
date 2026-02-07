# WiFi配网架构重构报告

## 🎯 重构目标

将WiFi配网从独立卡片改为设置中的一个功能，使架构更加合理。

---

## 📊 架构对比

### 旧架构（不合理）

```
卡片列表：
├── WiFiConfigCard（独立卡片）← 不合理
├── ImageCard
├── CalendarCard
└── ...
```

**问题**：
1. ❌ WiFi配网作为独立卡片，用户可以随时切换到它
2. ❌ 配网功能不应该和其他应用卡片平级
3. ❌ 卡片切换时会重复启动AP
4. ❌ 不符合用户习惯（配网通常在设置中）

---

### 新架构（合理）

```
卡片列表：
├── SettingsCard（设置卡片）
│   ├── WiFi配网 ← 菜单项
│   ├── 声音开关
│   ├── 固件版本
│   ├── 检查更新
│   └── 恢复出厂设置
├── ImageCard
├── CalendarCard
└── ...

工具类：
└── WiFiProvisioning（配网工具）
```

**优势**：
1. ✅ WiFi配网是设置中的一个菜单项
2. ✅ 只有在设置中点击"WiFi配网"才进入配网界面
3. ✅ 配网工具独立，可以被多个地方调用
4. ✅ 符合用户习惯和直觉

---

## 🔧 实现细节

### 1. WiFiProvisioning（配网工具类）

**文件**：
- `include/Network/WiFiProvisioning.h`
- `src/Network/WiFiProvisioning.cpp`

**特点**：
- 不再继承 `Card` 基类
- 改为独立的工具类
- 提供 `start()` / `stop()` / `update()` 方法
- 可以被 SettingsCard 或其他地方调用

**接口**：
```cpp
class WiFiProvisioning {
public:
    bool start();           // 启动配网
    void stop();            // 停止配网
    void update();          // 处理DNS和Web请求
    bool isProvisioning();  // 是否正在配网
    bool isConfigured();    // 配网是否完成
};
```

---

### 2. SettingsCard（设置卡片）

**文件**：
- `include/Cards/SettingsCard.h`
- `src/Cards/SettingsCard.cpp`

**功能**：
- 继承 `Card` 基类，是一个正常的卡片
- 提供菜单导航（上下滚动，短按选择）
- 包含5个菜单项

**菜单项**：
1. **WiFi配网** - 进入配网界面
2. **声音开关** - 切换声音开关（ON/OFF）
3. **固件版本** - 显示固件版本信息
4. **检查更新** - 检查并安装固件更新
5. **恢复出厂设置** - 清空所有配置

**状态机**：
```cpp
enum State {
    STATE_MENU,             // 菜单模式
    STATE_WIFI_PROVISIONING // WiFi配网模式
};
```

**交互流程**：
```
菜单模式：
  - 上下滚动：选择菜单项
  - 短按：执行选中的菜单项
  - 长按：退出设置（返回上一张卡片）

配网模式：
  - 显示二维码和WiFi信息
  - 处理Captive Portal
  - 长按：退出配网，返回菜单
  - 配网完成：自动重启设备
```

---

### 3. 首次启动逻辑

**旧逻辑**：
```cpp
if (config.isFirstBoot() || !config.hasWiFiConfig()) {
    // 直接进入 WiFiConfigCard
    cardManager->setCurrentCard(0);
}
```

**新逻辑**：
```cpp
if (config.isFirstBoot() || !config.hasWiFiConfig()) {
    // 进入 SettingsCard
    cardManager->setCurrentCard(0);
    // SettingsCard 检测到首次启动，自动进入配网模式
}
```

**SettingsCard 的自动配网逻辑**：
```cpp
void SettingsCard::onEnter() {
    // 如果是首次启动且没有WiFi配置，自动进入配网模式
    if (_config.isFirstBoot() || !_config.hasWiFiConfig()) {
        Serial.println("[SettingsCard] First boot, entering WiFi provisioning");
        _enterWiFiProvisioning();
    } else {
        _renderMenu();
    }
}
```

---

## 📝 文件变更清单

### 新增文件

1. ✅ `include/Network/WiFiProvisioning.h` - 配网工具类（头文件）
2. ✅ `src/Network/WiFiProvisioning.cpp` - 配网工具类（实现）
3. ✅ `include/Cards/SettingsCard.h` - 设置卡片（头文件）
4. ✅ `src/Cards/SettingsCard.cpp` - 设置卡片（实现）

### 删除文件（可选）

- `include/Cards/WiFiConfigCard.h` - 旧的配网卡片（不再使用）
- `src/Cards/WiFiConfigCard.cpp` - 旧的配网卡片（不再使用）

**注意**：旧文件可以保留作为参考，但不会被编译。

### 修改文件

1. ✅ `src/main.cpp` - 更新卡片初始化逻辑
   - 创建 `WiFiProvisioning` 工具
   - 创建 `SettingsCard` 卡片
   - 更新 `loop()` 中的配网处理逻辑

---

## 🧪 测试场景

### 场景1：首次启动（无WiFi配置）

**操作**：
1. 首次上传固件
2. 设备启动

**预期**：
```
[Setup] First boot or no WiFi config - entering settings
[CardManager] Switched to card: 设置
[SettingsCard] Entering settings
[SettingsCard] First boot, entering WiFi provisioning
[WiFiProvisioning] Starting provisioning...
[WiFiProvisioning] AP started: InfinityTag-xxxx
```

**屏幕显示**：
- 配网界面（二维码 + WiFi信息）

---

### 场景2：正常启动（已配置WiFi）

**操作**：
1. 设备已配置WiFi
2. 设备启动

**预期**：
```
[WiFi] Connected successfully
[WiFi] IP: 192.168.31.183
[Setup] Set current card to index: 0
[CardManager] Switched to card: 设置
[SettingsCard] Entering settings
[SettingsCard] Rendering menu
```

**屏幕显示**：
- 设置菜单界面

---

### 场景3：手动进入配网

**操作**：
1. 在设置菜单中
2. 滚动到"WiFi配网"
3. 短按确认

**预期**：
```
[SettingsCard] Execute menu item: 0
[SettingsCard] Entering WiFi provisioning
[WiFiProvisioning] Starting provisioning...
[WiFiProvisioning] AP started: InfinityTag-xxxx
```

**屏幕显示**：
- 配网界面（二维码 + WiFi信息）

---

### 场景4：退出配网

**操作**：
1. 在配网界面
2. 长按滚轮1秒

**预期**：
```
[InputManager] Long press detected
[SettingsCard] Exiting WiFi provisioning
[WiFiProvisioning] Stopping provisioning...
[WiFiProvisioning] AP stopped
[SettingsCard] Rendering menu
```

**屏幕显示**：
- 返回设置菜单

---

### 场景5：配网完成

**操作**：
1. 在配网界面
2. 手机连接AP
3. 输入WiFi信息并保存

**预期**：
```
[WiFiProvisioning] Received config submission
[WiFiProvisioning] SSID: YourWiFi
[ConfigManager] WiFi SSID set: YourWiFi
[ConfigManager] WiFi password set
```

**设备行为**：
- 自动重启
- 重启后连接到新WiFi

---

## 🎨 用户界面

### 设置菜单界面

```
┌─────────────────────────┐
│ Settings                │
├─────────────────────────┤
│ > WiFi Config           │
│   Sound            ON   │
│   Firmware    v1.0.0.1  │
│   Check Update          │
│   Factory Reset         │
└─────────────────────────┘
```

**交互**：
- `>` 表示当前选中项
- 上下滚动：移动选中项
- 短按：执行选中项

---

### 配网界面

```
┌─────────────────────────┐
│ WiFi Setup              │
├─────────────────────────┤
│ ████████  SSID:         │
│ ████████  InfinityTag-f3c│
│ ████████                │
│ ████████  Password:     │
│ ████████  12345678      │
│ ████████                │
│ ████████  Scan QR code  │
└─────────────────────────┘
```

**交互**：
- 扫描二维码连接AP
- 或手动连接AP
- 长按退出配网

---

## 📊 架构优势

### 1. 更清晰的职责分离

| 组件 | 职责 | 类型 |
|------|------|------|
| WiFiProvisioning | 配网功能实现 | 工具类 |
| SettingsCard | 设置界面和菜单 | 卡片 |
| CardManager | 卡片管理 | 管理器 |

### 2. 更好的可扩展性

**添加新的设置项**：
```cpp
// 只需在 SettingsCard 中添加菜单项
enum MenuItem {
    MENU_WIFI_CONFIG,
    MENU_SOUND_TOGGLE,
    MENU_NEW_SETTING,  // ← 新增
    MENU_COUNT
};
```

**复用配网工具**：
```cpp
// 其他地方也可以调用配网工具
WiFiProvisioning wifiProv(epd, config);
wifiProv.start();
```

### 3. 更符合用户习惯

- ✅ 配网在设置中，符合直觉
- ✅ 不会在卡片列表中看到配网卡片
- ✅ 首次启动自动进入配网
- ✅ 后续可以手动重新配网

---

## 🔄 迁移指南

### 从旧架构迁移

如果你的代码中使用了 `WiFiConfigCard`，需要做以下修改：

**1. 更新头文件引用**：
```cpp
// 旧代码
#include "Cards/WiFiConfigCard.h"

// 新代码
#include "Network/WiFiProvisioning.h"
#include "Cards/SettingsCard.h"
```

**2. 更新对象创建**：
```cpp
// 旧代码
WiFiConfigCard* wifiConfigCard = new WiFiConfigCard(epd, config);
cardManager->registerCard(wifiConfigCard);

// 新代码
WiFiProvisioning* wifiProv = new WiFiProvisioning(epd, config);
SettingsCard* settingsCard = new SettingsCard(epd, config, *wifiProv);
cardManager->registerCard(settingsCard);
```

**3. 更新主循环**：
```cpp
// 旧代码
if (wifiConfigCard && cardManager->getCurrentCardIndex() == 0) {
    wifiConfigCard->update();
}

// 新代码
if (settingsCard && cardManager->getCurrentCardIndex() == 0) {
    settingsCard->update();
}
```

---

## 🎉 重构完成

新架构的优势：
- ✅ WiFi配网不再是独立卡片
- ✅ 配网功能集成在设置中
- ✅ 更符合用户习惯
- ✅ 更好的可扩展性
- ✅ 更清晰的职责分离

请重新编译上传固件测试！

---

生成时间：2026-02-07
版本：v2.0
