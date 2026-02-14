# 如何进入 DemoCard

## 卡片顺序

系统启动后，卡片按以下顺序注册：

1. **索引 0：DemoCard（演示卡片）** ⭐ 默认卡片
2. **索引 1：SettingsCard（设置卡片）**
3. **索引 2+：Lua 卡片**（从 `/cards/` 目录加载）

## 方法一：自动进入（默认）

系统启动后会**自动进入 DemoCard**，因为它是第一张卡片（索引 0）。

### 操作步骤：
1. 上传固件：`pio run --target upload`
2. 等待系统启动完成
3. 自动显示 DemoCard 内容

## 方法二：手动切换卡片

如果当前不在 DemoCard，可以通过长按按钮切换：

### 操作步骤：
1. **长按按钮 1 秒** - 进入卡片切换模式
2. **旋转编码器** - 上下滚动选择卡片
   - 向上滚动：DemoCard（演示卡片）
   - 向下滚动：SettingsCard（设置）、Lua 卡片等
3. **短按按钮** - 确认选择，进入该卡片

### 切换界面显示：
```
┌────────────────────────┐
│   SettingsCard         │  ← 上方卡片
│   [图标]               │
├────────────────────────┤
│ ▶ DemoCard ◀          │  ← 当前选中（反色）
│   [图标]               │
├────────────────────────┤
│   Calendar Card        │  ← 下方卡片
│   [图标]               │
└────────────────────────┘
```

## DemoCard 功能

进入 DemoCard 后，你会看到：

### 显示内容：
```
┌────────────────────────┐
│   欢迎使用墨水屏        │
│   这是第二行文字        │
│   支持中文显示          │
│   可以添加背景图        │
│   按下按钮切换          │
│   旋转编码器滚动        │
│   Framebuffer系统       │
│   图层合成技术          │
│                        │
│      背景:关     ↓     │
└────────────────────────┘
```

### 操作说明：
- **旋转编码器**：上下滚动文本（8 行演示文本）
- **短按按钮**：切换背景图片开关（"背景:关" ↔ "背景:开"）
- **长按按钮**：退出到卡片切换模式

### 滚动指示器：
- **上箭头 ↑**：表示上方还有内容
- **下箭头 ↓**：表示下方还有内容

## 查看串口日志

通过串口监视器可以看到详细的运行日志：

```bash
pio device monitor
```

### 启动日志示例：
```
========================================
  INFINITY TAG V2 - Lua Card Engine
========================================
[Cards] Initializing cards...
[Cards] Registered DemoCard          ← DemoCard 已注册
[Cards] Registered SettingsCard
[Cards] Loaded Lua card: calendar.lua
[Cards] Registered 3 cards           ← 总共 3 张卡片
[Setup] Set current card to index: 0 ← 默认选择索引 0（DemoCard）
========================================
  SYSTEM READY
========================================
```

### DemoCard 运行日志：
```
[DemoCard] Entering demo card        ← 进入 DemoCard
[DemoCard] Line index: 0             ← 当前显示第 0 行
[DemoCard] Line index: 1             ← 滚动到第 1 行
[DemoCard] Background: ON            ← 背景图片开启
[DemoCard] Background image loaded   ← 背景图片加载成功
```

## 常见问题

### Q1: 启动后没有自动进入 DemoCard？

**可能原因**：
- 首次启动会自动进入 SettingsCard（配网模式）
- 配置文件中保存了上次选择的卡片索引

**解决方案**：
1. 长按按钮进入卡片切换模式
2. 旋转编码器选择 DemoCard
3. 短按按钮确认

### Q2: 看不到 DemoCard 选项？

**检查清单**：
1. 确认编译成功：`pio run`
2. 确认固件已上传：`pio run --target upload`
3. 查看串口日志确认注册成功

### Q3: DemoCard 显示空白？

**可能原因**：
- 字体文件未上传
- Framebuffer 分配失败

**解决方案**：
1. 上传文件系统：`pio run --target uploadfs`
2. 查看串口日志检查错误信息
3. 重启设备

### Q4: 背景图片不显示？

**原因**：背景图片文件不存在

**解决方案**：
1. 准备背景图片：`python tools/convert_image.py your_image.png demo_bg.bin`
2. 复制到 data 目录：`cp demo_bg.bin infinity-tag-esp32/data/images/`
3. 上传文件系统：`pio run --target uploadfs`
4. 在 DemoCard 中短按按钮切换到"背景:开"

## 修改默认卡片

如果想让系统启动后进入其他卡片，修改 `main.cpp`：

```cpp
// 在 setup() 函数的最后
if (cardManager->getCardCount() > 0) {
    // 修改这里的索引
    cardManager->setCurrentCard(0);  // 0=DemoCard, 1=SettingsCard, 2+=Lua卡片
}
```

## 卡片索引参考

| 索引 | 卡片名称 | 说明 |
|------|---------|------|
| 0 | DemoCard | 演示卡片（中文显示、背景图片） |
| 1 | SettingsCard | 设置卡片（WiFi配网、系统设置） |
| 2 | calendar.lua | 日历卡片（如果存在） |
| 3 | image.lua | 图片卡片（如果存在） |
| ... | 其他 Lua 卡片 | 按文件名排序 |

## 下一步

现在你已经知道如何进入 DemoCard 了！接下来可以：

1. ✅ 体验 DemoCard 的功能（滚动、背景切换）
2. ✅ 添加自己的背景图片
3. ✅ 修改 DemoCard 的文本内容
4. ✅ 创建自己的自定义卡片

祝你玩得开心！🎉
