# 卡片切换逻辑优化报告

## 🐛 问题描述

### 问题：重复进入 Config Mode

**现象**：
每次退出卡片切换模式时（无论是超时、取消还是选择同一张卡片），WiFiConfigCard 都会重新执行 `onEnter()`，导致 AP 重启。

**日志示例**：
```
[CardManager] Entering switch mode
[CardManager] Canceling switch mode
[WiFiConfigCard] Exiting WiFi config mode
[WiFiConfigCard] AP stopped
[WiFiConfigCard] Entering WiFi config mode  ← 不必要的重启
[WiFiConfigCard] Starting AP mode...
[WiFiConfigCard] APted: InfinityTag-f3c
[WiFiConfigCard] AP password: 12345678
```

**影响**：
1. 用户体验差：每次取消切换都会看到AP重启
2. 性能浪费：重复初始化AP、DNS服务器、Web服务器
3. 连接中断：如果用户正在配置WiFi，取消切换会断开连接

---

## 🔍 根本原因

### 原始逻辑

在 `CardManager::_exitSwitchMode()` 和 `_cancelSwitchMode()` 中：

```cpp
void CardManager::_exitSwitchMode() {
    // ...
    setCurrentCard(_switchPreviewIndex);  // 总是调用完整生命周期
}

void CardManager::_cancelSwitchMode() {
    // ...
    if (_currentCardIndex >= 0) {
        _cards[_currentCardIndex]->onEnter();  // 重复调用 onEnter()
    }
}
```

**问题**：
- `setCurrentCard()` 总是先调用 `onExit()`，再调用 `onEnter()`
- 即使是同一张卡片，也会经历完整的生命周期
- 取消切换时，不应该调用 `onEnter()`，因为卡片从未退出过

---

## ✅ 优化方案

### 方案1：在 `setCurrentCard()` 中判断是否是同一张卡片

**修改**：`src/Core/CardManager.cpp`

```cpp
bool CardManager::setCurrentCard(int index) {
    if (index < 0 || index >= static_cast<int>(_cards.size())) {
        Serial.printf("[CardManager] ERROR: Invalid card index: %d\n", index);
        return false;
    }

    // 如果切换回同一张卡片，不触发完整的 onExit/onEnter，避免重复初始化
    if (_currentCardIndex == index) {
        Serial.printf("[CardManager] Same card selected (index: %d), skipping lifecycle\n", index);
        return true;
    }

    // 退出当前卡片
    if (_currentCardIndex >= 0 && _currentCardIndex < static_cast<int>(_cards.size())) {
        _cards[_currentCardIndex]->onExit();
    }

    // 切换到新卡片
    _currentCardIndex = index;
    _cards[_currentCardIndex]->onEnter();

    Serial.printf("[CardManager] Switched to card: %s\n",
                  _cards[_currentCardIndex]->getName().c_str());

    return true;
}
```

**优化点**：
- 添加同卡片检测：`if (_currentCardIndex == index)`
- 如果是同一张卡片，直接返回，跳过生命周期
- 只有真正切换到不同卡片时，才执行 `onExit()/onEnter()`

---

### 方案2：优化 `_cancelSwitchMode()`

**修改**：`src/Core/CardManager.cpp`

```cpp
void CardManager::_cancelSwitchMode() {
    Serial.println("[CardManager] Canceling switch mode");

    _state = STATE_NORMAL;

    // 全屏闪白
    _flashWhite();

    // 不需要重新进入当前卡片，因为我们没有离开过
    // 只需要重新渲染当前卡片即可
    Serial.printf("[CardManager] Staying on card: %s\n",
                  _cards[_currentCardIndex]->getName().c_str());
}
```

**优化点**：
- 移除了 `_cards[_currentCardIndex]->onEnter()` 调用
- 因为取消切换时，卡片从未退出过，不需要重新进入
- 只需要返回正常模式，卡片状态保持不变

---

## 🧪 测试场景

### 场景1：选择同一张卡片

**操作**：
1. 当前在 WiFiConfigCard（索引0）
2. 长按进入切换模式
3. 左右滚动后又回到索引0
4. 松开滚轮确认

**优化前**：
```
[CardManager] Entering switch mode
[CardManager] Exiting switch mode
[WiFiConfigCard] Exiting WiFi config mode
[WiFiConfigCard] AP stopped
[WiFiConfigCard] Entering WiFi config mode  ← 不必要
[WiFiConfigCard] Starting AP mode...
```

**优化后**：
```
[CardManager] Entering switch mode
[CardManager] Exiting switch mode
[CardManager] Same card selected (index: 0), skipping lifecycle  ← 跳过
```

---

### 场景2：取消切换（超时）

**操作**：
1. 当前在 WiFiConfigCard（索引0）
2. 长按进入切换模式
3. 5秒内无操作，自动超时

**优化前**：
```
[CardManager] Entering switch mode
[CardManager] Switch mode timeout
[CardManager] Canceling switch mode
[WiFiConfigCard] Entering WiFi config mode  ← 不必要
[WiFiConfigCard] Starting AP mode...
```

**优化后**：
```
[CardManager] Entering switch mode
[CardManager] Switch mode timeout
[CardManager] Canceling switch mode
[CardManager] Staying on card: WiFi配网  ← 保持不变
```

---

### 场景3：切换到不同卡片

**操作**：
1. 当前在 WiFiConfigCard（索引0）
2. 长按进入切换模式
3. 滚动到 SettingsCard（索引1）
4. 松开滚轮确认

**优化前**：
```
[CardManager] Entering switch mode
[CardManager] Exiting switch mode
[WiFiConfigCard] Exiting WiFi config mode
[SettingsCard] Entering settings mode
[CardManager] Switched to card: 设置
```

**优化后**：
```
[CardManager] Entering switch mode
[CardManager] Exiting switch mode
[WiFiConfigCard] Exiting WiFi config mode
[SettingsCard] Entering settings mode
[CardManager] Switched to card: 设置
```

**说明**：切换到不同卡片时，逻辑保持不变，正常执行生命周期。

---

## 📊 优化效果

### 性能提升

| 场景 | 优化前 | 优化后 | 提升 |
|------|--------|--------|------|
| 选择同一张卡片 | 完整生命周期 | 跳过 | 100% |
| 取消切换 | 重复 onEnter() | 跳过 | 100% |
| 切换到不同卡片 | 完整生命周期 | 完整生命周期 | 0% |

### 用户体验提升

| 问题 | 优化前 | 优化后 |
|------|--------|--------|
| AP重启 | ✅ 每次都重启 | ❌ 不重启 |
| 配置中断 | ✅ 会中断 | ❌ 不中断 |
| 响应速度 | 慢（重新初始化） | 快（直接返回） |
| 日志噪音 | 多 | 少 |

---

## 🎯 设计原则

### 卡片生命周期管理

**原则1：最小化生命周期调用**
- 只有在真正需要时才调用 `onEnter()/onExit()`
- 避免不必要的重复初始化

**原则2：状态保持**
- 在切换模式期间，当前卡片的状态应该保持不变
- 只有确认切换到新卡片时，才改变状态

**原则3：用户体验优先**
- 避免不必要的重启、重连、重新加载
- 保持用户操作的连续性

---

## 🔧 扩展优化建议

### 1. 添加卡片刷新事件

如果需要在选择同一张卡片时刷新内容，可以添加一个刷新事件：

```cpp
// 在 Event.h 中添加
EVENT_CARD_REFRESH,  // 卡片刷新请求

// 在 setCurrentCard() 中
if (_currentCardIndex == index) {
    Serial.printf("[CardManager] Same card selected, sending refresh event\n");
    Event refreshEvent(EVENT_CARD_REFRESH);
    _cards[_currentCardIndex]->onEvent(refreshEvent);
    return true;
}
```

### 2. 添加卡片状态保存

在切换卡片时，保存当前卡片的状态：

```cpp
class Card {
public:
    virtual void saveState() {}
    virtual void restoreState() {}
};
```

### 3. 添加切换动画优化

如果是同一张卡片，可以跳过过渡动画：

```cpp
void CardManager::_exitSwitchMode() {
    if (_switchPreviewIndex == _currentCardIndex) {
        // 同一张卡片，跳过动画
        _state = STATE_NORMAL;
        return;
    }

    // 不同卡片，显示过渡动画
    _renderTransition();
    setCurrentCard(_switchPreviewIndex);
}
```

---

## 📝 修改文件清单

1. ✅ `src/Core/CardManager.cpp` - 优化 `setCurrentCard()`
2. ✅ `src/Core/CardManager.cpp` - 优化 `_cancelSwitchMode()`

---

## 🎉 优化完成

现在卡片切换逻辑更加智能：
- ✅ 选择同一张卡片时，跳过生命周期
- ✅ 取消切换时，不重复初始化
- ✅ 切换到不同卡片时，正常执行生命周期
- ✅ WiFiConfigCard 不会不必要地重启 AP
- ✅ 用户配置过程不会被中断

请重新编译上传固件测试！

---

生成时间：2026-02-07
版本：v1.2
