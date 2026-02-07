# 卡片图标问题修复报告

## 🐛 问题描述

### 问题1：缺少卡片图标文件
```
[E][vfs_api.cpp:105] open(): /littlefs/icons/card_wifi.bin does not exist
[CardManager] Logo not found: /icons/card_wifi.bin
```

### 问题2：无效的卡片索引
```
[CardManager] ERROR: Invalid card index: -1
```

---

## ✅ 修复方案

### 修复1：在正常模式下设置默认卡片

**文件**：`src/main.cpp`

**问题原因**：
- 在配网模式下，代码调用了 `cardManager->setCurrentCard(0)`
- 但在正常模式下，没有设置当前卡片
- 导致 `currentCardIndex` 一直是初始值 -1

**修复**：在 `setup()` 函数的正常模式分支中添加：

```cpp
// 设置默认卡片（如果有卡片的话）
if (cardManager->getCardCount() > 0) {
    // 从配置中读取上次选中的卡片索引
    int lastCardIndex = config.getCurrentCardIndex();
    if (lastCardIndex >= 0 && lastCardIndex < cardManager->getCardCount()) {
        cardManager->setCurrentCard(lastCardIndex);
    } else {
        // 默认选择第一张卡片
        cardManager->setCurrentCard(0);
    }
    Serial.printf("[Setup] Set current card to index: %d\n", cardManager->getCurrentCardIndex());
}
```

**功能**：
- 优先恢复上次选中的卡片（记忆功能）
- 如果上次的索引无效，则选择第一张卡片
- 如果没有卡片，则不设置（避免错误）

---

### 修复2：添加防御性检查

**文件**：`src/Core/CardManager.cpp`

**修改1**：在 `_handleNormalEvent()` 开头添加检查

```cpp
void CardManager::_handleNormalEvent(const Event& event) {
    // 检查是否有有效的当前卡片
    if (_currentCardIndex < 0 || _currentCardIndex >= static_cast<int>(_cards.size())) {
        Serial.printf("[CardManager] WARNING: No valid current card (index: %d), ignoring event\n",
                      _currentCardIndex);
        return;
    }

    switch (event.type) {
        // ...
    }
}
```

**修改2**：在 `render()` 中改进错误信息

```cpp
void CardManager::render(bool wifiConnected, int batteryLevel) {
    // ...

    if (_currentCardIndex < 0 || _currentCardIndex >= static_cast<int>(_cards.size())) {
        Serial.printf("[CardManager] ERROR: Invalid current card index: %d (total cards: %d)\n",
                      _currentCardIndex, _cards.size());
        return;
    }

    // ...
}
```

**功能**：
- 在处理事件前检查卡片索引是否有效
- 在渲染前检查卡片索引是否有效
- 提供更详细的错误信息（包含总卡片数）
- 避免访问无效索引导致崩溃

---

### 修复3：创建默认图标

**工具**：`tools/generate_icons.py`

**功能**：
- 生成48x48的1-bit位图图标
- 支持自定义文字
- 输出二进制格式（.bin文件）

**使用方法**：

```bash
# 1. 安装依赖
pip install Pillow

# 2. 生成默认图标
cd infinity-tag-esp32
python tools/generate_icons.py

# 3. 上传到设备
pio run -t uploadfs
```

**生成的图标**：
- `data/icons/card_wifi.bin` - WiFi配网卡片
- `data/icons/card_settings.bin` - 设置卡片
- `data/icons/card_image.bin` - 图片卡片
- `data/icons/card_calendar.bin` - 黄历卡片
- `data/icons/card_muyu.bin` - 木鱼卡片
- `data/icons/card_default.bin` - 默认卡片

**注意**：
- 图标文件是可选的
- 如果图标不存在，CardManager会绘制一个方框作为占位符
- 所以即使不上传图标，系统也能正常工作

---

## 🧪 测试步骤

### 1. 编译并上传固件

```bash
cd infinity-tag-esp32
pio run -t upload
```

### 2. 观察启动日志

**配网模式**（首次启动）：
```
[Setup] First boot or no WiFi config - entering config mode
[WiFiConfigCard] Entering WiFi config mode
[CardManager] Switched to card: WiFi配网
```

**正常模式**（已配置WiFi）：
```
[WiFi] Connected successfully
[WiFi] IP: 192.168.31.183
[Setup] Set current card to index: 0  ← 新增日志
[CardManager] Switched to card: WiFi配网
```

### 3. 测试卡片切换

1. **长按滚轮1秒**
   - 应该看到：`[InputManager] Long press detected`
   - 应该看到：`[CardManager] Entering switch mode`
   - 屏幕显示卡片选择界面

2. **左右滚动**
   - 选中的卡片应该反色显示
   - 如果有图标，显示图标；否则显示方框

3. **松开滚轮**
   - 应该看到：`[CardManager] Exiting switch mode`
   - 应该看到：`[CardManager] Switched to card: xxx`
   - 屏幕显示新卡片内容

### 4. 测试记忆功能

1. 切换到某张卡片（如索引1）
2. 重启设备
3. 设备应该自动恢复到索引1的卡片

---

## 📊 预期日志输出

### 正常启动（已配置WiFi）

```
========================================
  INFINITY TAG V2 - Lua Card Engine
========================================
[CRITICAL] PWR_IO set to HIGH - Power hold enabled
Firmware: v1.0.0.1
[OK] Hardware initialized
[LittleFS] Mounted successfully
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
[WiFi] Connecting to: SumHome
[WiFi] Connected successfully
[WiFi] IP: 192.168.31.183
[Setup] Set current card to index: 0  ← 新增
[CardManager] Switched to card: WiFi配网  ← 新增
========================================
  SYSTEM READY
========================================
```

### 长按切换卡片

```
[InputManager] Long press detected
[CardManager] Entering switch mode
[CardManager] Rendering card switch UI
[CardManager] Logo not found: /icons/card_wifi.bin  ← 如果没有图标
[CardManager] Exiting switch mode
[CardManager] Switched to card: WiFi配网
```

---

## 🎯 修复效果

### 修复前
- ❌ 正常模式下 `currentCardIndex = -1`
- ❌ 处理事件时崩溃
- ❌ 渲染时崩溃
- ⚠️ 缺少图标文件

### 修复后
- ✅ 正常模式下自动设置默认卡片
- ✅ 记忆上次选中的卡片
- ✅ 防御性检查避免崩溃
- ✅ 提供图标生成工具
- ✅ 图标缺失时使用占位符

---

## 📝 修改文件清单

1. ✅ `src/main.cpp` - 在正常模式下设置默认卡片
2. ✅ `src/Core/CardManager.cpp` - 添加防御性检查
3. ✅ `tools/generate_icons.py` - 图标生成工具（新增）

---

## 🔧 可选：上传图标文件

如果想要显示实际的图标而不是占位符：

```bash
# 1. 生成图标
python tools/generate_icons.py

# 2. 配置 platformio.ini（如果还没有）
# 添加以下内容：
# board_build.filesystem = littlefs
# board_build.partitions = default_16MB.csv

# 3. 上传文件系统
pio run -t uploadfs
```

**注意**：
- `uploadfs` 会上传 `data/` 目录下的所有文件到 LittleFS
- 上传后设备会自动重启
- 图标文件总大小约 1.7KB（6个图标 × 288字节）

---

## 🎉 修复完成

现在系统应该完全正常工作：
- ✅ WiFi连接正常
- ✅ 长按检测正常
- ✅ 卡片切换正常
- ✅ 无效索引已修复
- ✅ 图标缺失有占位符

请重新编译上传固件测试！

---

生成时间：2026-02-07
版本：v1.1
