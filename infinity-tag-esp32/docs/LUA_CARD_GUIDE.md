# 📘 Lua 卡片完整开发指南

欢迎使用 Infinity Tag ESP32 的 Lua 卡片系统！本指南将带您从零开始，学习如何开发、调试和部署自己的 Lua 卡片。

---

## 📋 目录

- [第一部分：快速开始](#第一部分快速开始)
- [第二部分：Lua 卡片基础](#第二部分lua-卡片基础)
- [第三部分：API 参考手册](#第三部分api-参考手册)
- [第四部分：开发实战](#第四部分开发实战)
- [第五部分：部署和调试](#第五部分部署和调试)
- [第六部分：最佳实践](#第六部分最佳实践)
- [第七部分：故障排查](#第七部分故障排查)

---

## 🎯 文档目标

阅读本文档后，您将能够：
- ✅ **5 分钟内**：运行 Hello World 示例
- ✅ **30 分钟内**：理解所有 API 的用法
- ✅ **1 小时内**：编写并部署自己的第一个卡片
- ✅ **2 小时内**：实现一个包含网络请求的复杂卡片

---

## 第一部分：快速开始

### 🚀 5 分钟上手

#### Hello World 示例

这是最简单的 Lua 卡片，只需 15 行代码：

```lua
-- 元数据
CARD_NAME = "Hello"
CARD_CATEGORY = "其他"
CARD_LOGO = "/icons/card_default.bin"
CARD_ORDER = 1
CARD_ENABLED = true

-- 初始化
function onInit()
    eink.clear()
    eink.drawStr(50, 40, "Hello World!")
    eink.refreshDeep()
end

-- 其他必需的回调函数（可以为空）
function onExit() end
function onLoop() end
function onBtnPress() end
```

**这个卡片做了什么？**
1. 定义卡片元数据（名称、分类、图标等）
2. 在 `onInit()` 中清空屏幕
3. 在坐标 (50, 40) 绘制 "Hello World!" 文本
4. 使用深度刷新显示内容

---

### 🛠️ 开发环境准备

#### 必需工具

| 工具 | 版本 | 用途 | 安装命令 |
|------|------|------|----------|
| **PlatformIO** | 最新版 | 编译和上传固件 | `pip install platformio` |
| **Python** | 3.8+ | 运行工具脚本 | [python.org](https://python.org) |
| **Git** | 2.0+ | 版本控制 | [git-scm.com](https://git-scm.com) |
| **VSCode** | 推荐 | 代码编辑器 | [code.visualstudio.com](https://code.visualstudio.com) |

#### 安装步骤

```bash
# 1. 克隆项目
git clone <repository-url>
cd infinity-tag-esp32

# 2. 安装 PlatformIO
pip install platformio

# 3. 安装依赖库
pio lib install

# 4. 验证环境
pio run  # 编译项目
```

---

### 📝 第一个卡片：完整流程

#### 步骤 1：创建卡片文件

在 `data/cards/` 目录下创建 `hello.lua`：

```bash
# Windows
notepad infinity-tag-esp32\data\cards\hello.lua

# Linux/Mac
nano infinity-tag-esp32/daello.lua
```

#### 步骤 2：编写卡片代码

复制以下代码到 `hello.lua`：

```lua
-- 元数据
CARD_NAME = "Hello"
CARD_CATEGORY = "其他"
CARD_LOGO = "/icons/card_default.bin"
CARD_ORDER = 1
CARD_ENABLED = true

-- 状态变量
local clickCount = 0

-- 初始化
function onInit()
    sys.log("Hello card initialized")
    clickCount = 0
    _render()
end

-- 退出
function onExit()
    sys.log("Hello card exited")
end

-- 主循环
function onLoop()
    -- 暂时不需要
end

-- 按键事件
function onBtnPress()
    clickCount = clickCount + 1
    sys.log("Button pressed, count: " .. clickCount)
    _render()
end

-- 渲染函数
function _render()
    eink.clear()
    eink.drawStr(30, 20, "Hello World!")
    eink.drawStr(30, 40, "Clicks: " .. clickCount)
    eink.drawStr(30, 60, "Press button to count")

    if clickCount == 0 then
        eink.refreshDeep()  -- 首次使用深度刷新
    else
        eink.refreshPartial()  -- 更新使用局部刷新
    end
end
```

#### 步骤 3：上传到设备

```bash
# 1. 连接设备到 USB

# 2. 上传文件系统（包含 Lua 卡片）
pio run -t uploadfs

# 3. 等待上传完成
# 输出示例：
# Writing at 0x00110000... (100 %)
# Wrote 1048576 bytes at 0x00110000
```

#### 步骤 4：查看运行效果

```bash
# 打开串口监视器
pio device monitor

# 你会看到类似输出：
# [Lua] Hello card initialized
# [Lua] Button pressed, count: 1
# [Lua] Button pressed, count: 2
```

**设备操作**：
1. 旋转编码器切换到 "Hello" 卡片
2. 按下按键，计数器会增加
3. 屏幕会显示点击次数

---

### 🎉 恭喜！

您已经完成了第一个 Lua 卡片的开发和部署！

**接下来**：
- 继续阅读 [第二部分](#第二部分lua-卡片基础) 了解 Lua 卡片的工作原理
- 查看 [第三部分](#第三部分api-参考手册) 学习所有可用的 API
- 参考 [第四部分](#第四部分开发实战) 的完整示例

---

## 第二部分：Lua 卡片基础

### 📚 卡片生命周期

Lua 卡片有明确的生命周期，由系统自动调用以下回调函数：

```
用户切换到卡片
    ↓
onInit()        ← 初始化（仅调用一次）
    ↓
onLoop()        ← 主循环（最高 10Hz）
    ↓           ← 用户交互触发事件
onBtnPress()    ← 按键短按
onBtnLong()     ← 按键长按
onEncoderCW()   ← 编码器顺时针
onEncoderCCW()  ← 编码器逆时针
onShake()       ← 设备摇晃
    ↓
用户切换到其他卡片
    ↓
onExit()        ← 清理资源（仅调用一次）
```

---

#### `onInit()` - 初始化

**调用时机**：用户切换到该卡片时调用一次

**用途**：
- 初始化状态变量
- 从 NVS 加载保存的数据
- 绘制初始界面
- **建议使用 `eink.refreshDeep()` 彻底清除残影**

```lua
function onInit()
    sys.log("Card initialized")

    -- 初始化状态
    counter = 0

    -- 从 NVS 加载数据
    local saved = nvs.get("my_counter")
    if saved ~= "" then
        counter = tonumber(saved) or 0
    end

    -- 绘制初始界面
    eink.clear()
    eink.drawStr(10, 10, "Counter: " .. counter)
    eink.refreshDeep()  -- 首次进入建议使用深度刷新
end
```

---

#### `onExit()` - 退出清理

**调用时机**：用户切换到其他卡片时调用一次

**用途**：
- 保存状态到 NVS
- 释放资源
- 记录日志

```lua
function onExit()
    sys.log("Card exited")

    -- 保存状态到 NVS
    nvs.set("my_counter", tostring(counter))

    -- 清理资源（如果有）
end
```

---

#### `onLoop()` - 主循环

**调用时机**：卡片激活期间持续调用

**调用频率**：最高 10Hz（每 100ms 一次）

**用途**：
- 定期更新界面（如时钟）
- 轮询状态变化
- **⚠️ 避免阻塞操作**（不要使用 `sys.delay()`）

```lua
local lastUpdate = 0

function onLoop()
    local now = sys.millis()

    -- 每 5 秒更新一次
    if now - lastUpdate > 5000 then
        lastUpdate = now

        -- 更新界面
        eink.clear()
        eink.drawStr(10, 10, "Time: " .. now)
        eink.refreshPartial()
    end
end
```

**⚠️ 注意事项**：
- `onLoop()` 最高 10Hz，不要期望更高频率
- 避免在 `onLoop()` 中使用 `sys.delay()`，会阻塞整个系统
- 使用 `sys.millis()` 实现定时逻辑

---

### 🏷️ 卡片元数据

每个 Lua 卡片必须在文件开头定义以下全局变量：

```lua
-- 元数据（必需）
CARD_NAME = "我的卡片"                    -- 卡片名称（显示在切换界面）
CARD_CATEGORY = "展示"                    -- 分类（展示/决策站/其他）
CARD_LOGO = "/icons/card_default.bin"    -- 图标路径（48x48 像素）
CARD_ORDER = 100                          -- 排序权重（越小越靠前）
CARD_ENABLED = true                       -- 是否启用（false 则不加载）
```

#### 元数据字段说明

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `CARD_NAME` | string | ✅ | 卡片名称，显示在卡片切换界面 |
| `CARD_CATEGORY` | string | ✅ | 分类：`"展示"` / `"决策站"` / `"其他"` |
| `CARD_LOGO` | string | ✅ | 图标文件路径，48x48 像素二进制格式 |
| `CARD_ORDER` | number | ✅ | 排序权重，数字越小越靠前（0-999） |
| `CARD_ENABLED` | boolean | ✅ | 是否启用，`false` 则不加载该卡片 |

#### 示例

```lua
-- 天气卡片
CARD_NAME = "天气"
CARD_CATEGORY = "展示"
CARD_LOGO = "/icons/card_weather.bin"
CARD_ORDER = 10
CARD_ENABLED = true

-- 决策卡片
CARD_NAME = "抛硬币"
CARD_CATEGORY = "决策站"
CARD_LOGO = "/icons/card_coin.bin"
CARD_ORDER = 50
CARD_ENABLED = true

-- 禁用的卡片
CARD_NAME = "测试"
CARD_CATEGORY = "其他"
CARD_LOGO = "/icons/card_default.bin"
CARD_ORDER = 999
CARD_ENABLED = false  -- 不会被加载
```

---

### 🎮 事件处理

Lua 卡片支持 6 种用户交互事件：

#### 1. `onBtnPress()` - 按键短按

**触发条件**：按键按下后快速释放（< 1 秒）

```lua
function onBtnPress()
    counter = counter + 1
    sys.log("Button pressed, counter: " .. counter)

    eink.clear()
    eink.drawStr(10, 10, "Count: " .. counter)
    eink.refreshPartial()
end
```

---

#### 2. `onBtnLong()` - 按键长按

**触发条件**：按键按下超过 1 秒

**常见用途**：重置、确认操作

```lua
function onBtnLong()
    counter = 0
    sys.log("Long press - counter reset")

    eink.clear()
    eink.drawStr(10, 10, "Reset!")
    eink.refreshPartial()
end
```

---

#### 3. `onEncoderCW()` - 编码器顺时针旋转

**触发条件**：旋转编码器顺时针方向

**常见用途**：增加数值、向下滚动

```lua
function onEncoderCW()
    value = value + 1
    sys.log("Encoder CW, value: " .. value)
    _updateDisplay()
end
```

---

#### 4. `onEncoderCCW()` - 编码器逆时针旋转

**触发条件**：旋转编码器逆时针方向

**常见用途**：减少数值、向上滚动

```lua
function onEncoderCCW()
    value = value - 1
    sys.log("Encoder CCW, value: " .. value)
    _updateDisplay()
end
```

---

#### 5. `onShake()` - 设备摇晃

**触发条件**：检测到设备被摇晃（振动开关触发）

**常见用途**：随机功能、刷新数据

```lua
function onShake()
    sys.log("Device shaken!")

    -- 随机数
    math.randomseed(sys.millis())
    randomValue = math.random(1, 100)

    eink.clear()
    eink.drawStr(10, 10, "Random: " .. randomValue)
    eink.refreshPartial()
end
```

---

#### 事件处理完整示例

```lua
-- 元数据
CARD_NAME = "交互演示"
CARD_CATEGORY = "其他"
CARD_LOGO = "/icons/card_default.bin"
CARD_ORDER = 1
CARD_ENABLED = true

-- 状态变量
local counter = 0
local mode = "normal"

function onInit()
    counter = 0
    mode = "normal"
    _render()
end

function onExit()
    nvs.set("demo_counter", tostring(counter))
end

function onLoop()
    -- 暂时不需要
end

-- 短按：增加计数
function onBtnPress()
    counter = counter + 1
    _render()
end

-- 长按：重置计数
function onBtnLong()
    counter = 0
    _render()
end

-- 顺时针：切换模式
function onEncoderCW()
    if mode == "normal" then
        mode = "fast"
    else
        mode = "normal"
    end
    _render()
end

-- 逆时针：切换模式
function onEncoderCCW()
    if mode == "fast" then
        mode = "normal"
    else
        mode = "fast"
    end
    _render()
end

-- 摇晃：随机计数
function onShake()
    math.randomseed(sys.millis())
    counter = math.random(0, 100)
    _render()
end

-- 渲染函数
function _render()
    eink.clear()
    eink.drawStr(10, 10, "Counter: " .. counter)
    eink.drawStr(10, 30, "Mode: " .. mode)
    eink.drawStr(10, 50, "Press: +1")
    eink.drawStr(10, 65, "Long: Reset")
    eink.drawStr(10, 80, "Rotate: Mode")
    eink.drawStr(10, 95, "Shake: Random")

    if counter == 0 then
        eink.refreshDeep()
    else
        eink.refreshPartial()
    end
end
```

---

### 🖥️ 渲染机制

#### 屏幕规格

| 参数 | 值 |
|------|-----|
| **分辨率** | 212 × 104 像素 |
| **颜色** | 黑白（1-bit） |
| **坐标系** | 左上角 (0, 0)，右下角 (211, 103) |

#### 三种刷新模式

Lua 卡片支持 3 种刷新模式，根据场景选择：

| 刷新模式 | 函数 | 速度 | 残影 | 闪烁 | 使用场景 |
|---------|------|------|------|------|----------|
| **局部刷新** | `eink.refreshPartial()` | 最快 (~100ms) | 可能有 | 无 | 频繁更新（计数器、时钟） |
| **全屏刷新** | `eink.refresh()` | 中等 (~300ms) | 较少 | 无 | 普通更新 |
| **深度刷新** | `eink.refreshDeep()` | 最慢 (~900ms) | 无 | 3次 | 首次进入、显示图片 |

#### 刷新策略建议

```lua
function onInit()
    -- 首次进入：使用深度刷新，彻底清除残影
    eink.clear()
    eink.drawStr(10, 10, "Welcome!")
    eink.refreshDeep()
end

function onBtnPress()
    -- 频繁更新：使用局部刷新，速度最快
    counter = counter + 1
    eink.clear()
    eink.drawStr(10, 10, "Count: " .. counter)
    eink.refreshPartial()
end

function onShake()
    -- 重要更新：使用全屏刷新，平衡速度和质量
    eink.clear()
    eink.drawStr(10, 10, "Shaken!")
    eink.refresh()
end
```

#### 刷新频率限制

- `onLoop()` 最高 10Hz（每 100ms）
- 建议每次刷新间隔至少 100ms
- 过于频繁的刷新会导致：
  - 屏幕闪烁
  - 残影累积
  - 性能下降

---

### 📊 状态管理

#### 局部变量 vs 全局变量

```lua
-- ❌ 错误：使用全局变量（会污染全局命名空间）
counter = 0

-- ✅ 正确：使用 local 局部变量
local counter = 0
local lastUpdate = 0
local mode = "normal"
```

#### 持久化存储

使用 NVS（Non-Volatile Storage）保存数据，重启后保留：

```lua
function onInit()
    -- 从 NVS 加载数据
    local saved = nvs.get("my_key")
    if saved ~= "" then
        counter = tonumber(saved) or 0
        sys.log("Loaded: " .. counter)
    end
end

function onExit()
    -- 保存数据到 NVS
    nvs.set("my_key", tostring(counter))
    sys.log("Saved: " .. counter)
end
```

**⚠️ 注意**：
- NVS 只能存储字符串，需要使用 `tostring()` 和 `tonumber()` 转换
- 所有 Lua 卡片共享同一个 NVS 命名空间，建议使用唯一的键名（如 `"weather_city"`）

---

### 🎯 小结

**第二部分关键要点**：

1. **生命周期**：`onInit()` → `onLoop()` → 事件回调 → `onExit()`
2. **元数据**：5 个必需字段（NAME, CATEGORY, LOGO, ORDER, ENABLED）
3. **事件处理**：6 种事件（短按、长按、编码器、摇晃）
4. **刷新策略**：首次 DEEP，更新 Partial，重要 Full
5. **状态管理**：使用 local 变量，NVS 持久化

**接下来**：
- 查看 [第三部分：API 参考手册](#第三部分api-参考手册) 学习所有可用的 API 函数

---

## 第三部分：API 参考手册

Lua 卡片引擎提供了 **5 个命名空间，共 25+ 个函数**：

| 命名空间 | 功能 | 函数数量 |
|---------|------|---------|
| `eink.*` | 墨水屏绘图 | 12 个 |
| `nvs.*` | 永久存储 | 3 个 |
| `http.*` | 网络请求 | 3 个 |
| `hw.*` | 硬件控制 | 3 个 |
| `sys.*` | 系统工具 | 3 个 |

---

### 🖥️ 墨水屏绘图 API (`eink.*`)

#### 屏幕信息

##### `eink.getWidth()`

获取屏幕宽度

**返回值**：`number` - 屏幕宽度（212 像素）

```lua
local w = eink.getWidth()  -- 212
sys.log("Width: " .. w)
```

---

##### `eink.getHeight()`

获取屏幕高度

**返回值**：`number` - 屏幕高度（104 像素）

```lua
local h = eink.getHeight()  -- 104
sys.log("Height: " .. h)
```

---

#### 基础绘图

##### `eink.clear()`

清空屏幕（填充白色）

**参数**：无

**返回值**：无

```lua
eink.clear()  -- 清空屏幕
```

**注意**：调用 `clear()` 后需要调用刷新函数才能显示

---

##### `eink.drawPixel(x, y, color)`

绘制单个像素点

**参数**：
- `x` (number): X 坐标 (0-211)
- `y` (number): Y 坐标 (0-103)
- `color` (number, 可选): 颜色，0=黑色，非0=白色，默认 0

**返回值**：无

```lua
-- 绘制黑色像素
eink.drawPixel(50, 50, 0)

-- 绘制白色像素
eink.drawPixel(51, 50, 1)

-- 默认黑色
eink.drawPixel(52, 50)
```

---

##### `eink.drawLine(x1, y1, x2, y2)`

绘制直线

**参数**：
- `x1, y1` (number): 起点坐标
- `x2, y2` (number): 终点坐标

**返回值**：无

```lua
-- 绘制水平线
eink.drawLine(10, 50, 200, 50)

-- 绘制垂直线
eink.drawLine(100, 10, 100, 90)

-- 绘制斜线
eink.drawLine(10, 10, 200, 90)
```

---

##### `eink.drawRect(x, y, w, h)`

绘制矩形边框

**参数**：
- `x, y` (number): 左上角坐标
- `w, h` (number): 宽度和高度

**返回值**：无

```lua
-- 绘制边框
eink.drawRect(10, 10, 100, 50)

-- 绘制屏幕边框
eink.drawRect(0, 0, 212, 104)
```

---

##### `eink.fillRect(x, y, w, h)`

绘制填充矩形（黑色）

**参数**：
- `x, y` (number): 左上角坐标
- `w, h` (number): 宽度和高度

**返回值**：无

```lua
-- 绘制黑色矩形
eink.fillRect(10, 10, 50, 30)

-- 绘制进度条
local progress = 0.6  -- 60%
eink.drawRect(10, 50, 100, 10)  -- 边框
eink.fillRect(10, 50, 100 * progress, 10)  -- 填充
```

---

#### 文本绘制

##### `eink.drawStr(x, y, text)`

绘制 ASCII 文本（使用默认字体）

**参数**：
- `x, y` (number): 文本起始坐标
- `text` (string): 要绘制的文本

**返回值**：无

**字体**：GFX 默认字体（5x7 像素）

```lua
-- 绘制文本
eink.drawStr(10, 10, "Hello World")

-- 绘制数字
eink.drawStr(10, 30, "Counter: " .. counter)

-- 绘制多行
eink.drawStr(10, 10, "Line 1")
eink.drawStr(10, 25, "Line 2")
eink.drawStr(10, 40, "Line 3")
```

---

##### `eink.drawChinese(x, y, text)`

绘制中文文本（使用自定义中文字体）

**参数**：
- `x, y` (number): 文本起始坐标
- `text` (string): UTF-8 编码的中文文本

**返回值**：无

**字体**：16px 中文字体（汇文仿宋）

```lua
-- 绘制中文
eink.drawChinese(60, 10, "今日黄历")

-- 绘制中英文混合
eink.drawChinese(10, 30, "温度: 25°C")

-- 绘制多行中文
eink.drawChinese(10, 10, "宜: 出行 祭祀")
eink.drawChinese(10, 30, "忌: 动土 嫁娶")
```

**注意**：
- 中文字符宽度为 16px
- 全角标点（如 `：`）宽度为 16px
- 半角标点（如 `:`）宽度为 8px

---

#### 屏幕刷新

##### `eink.refreshPartial()`

局部刷新（快速模式）

**参数**：无

**返回值**：无

**特点**：
- 速度最快（~100ms）
- 无闪烁
- 可能有残影

**使用场景**：频繁更新（计数器、时钟）

```lua
function onBtnPress()
    counter = counter + 1
    eink.clear()
    eink.drawStr(10, 10, "Count: " .. counter)
    eink.refreshPartial()  -- 快速更新
end
```

---

##### `eink.refresh()`

全屏刷新（标准模式）

**参数**：无

**返回值**：无

**特点**：
- 速度中等（~300ms）
- 无闪烁
- 残影较少

**使用场景**：普通更新

```lua
function onShake()
    eink.clear()
    eink.drawStr(10, 10, "Shaken!")
    eink.refresh()  -- 标准刷新
end
```

---

##### `eink.refreshDeep()`

深度刷新（DEEP 模式）

**参数**：无

**返回值**：无

**特点**：
- 速度最慢（~900ms）
- 闪烁 3 次
- 完全消除残影

**使用场景**：
- 首次进入卡片（`onInit()`）
- 显示图片
- 需要彻底清除残影

```lua
function onInit()
    eink.clear()
    eink.drawStr(10, 10, "Welcome!")
    eink.refreshDeep()  -- 首次进入使用深度刷新
end
```

---

### 💾 永久存储 API (`nvs.*`)

使用 ESP32 的 NVS (Non-Volatile Storage) 存储键值对，数据在重启后保留。

**命名空间**：所有 Lua 卡片共享 `"lua_cards"` 命名空间

---

##### `nvs.set(key, value)`

存储键值对

**参数**：
- `key` (string): 键名
- `value` (string): 值（必须是字符串）

**返回值**：无

```lua
-- 存储字符串
nvs.set("username", "Alice")

-- 存储数字（需要转换为字符串）
nvs.set("counter", tostring(123))

-- 存储布尔值
nvs.set("enabled", tostring(true))
```

**注意**：
- 只能存储字符串，需要使用 `tostring()` 转换
- 建议使用唯一的键名（如 `"weather_city"`）避免冲突

---

##### `nvs.get(key)`

读取存储的值

**参数**：
- `key` (string): 键名

**返回值**：`string` - 存储的值，如果键不存在返回空字符串 `""`

```lua
-- 读取字符串
local username = nvs.get("username")
if username ~= "" then
    sys.log("Username: " .. username)
end

-- 读取数字
local saved = nvs.get("counter")
if saved ~= "" then
    counter = tonumber(saved) or 0
end

-- 读取布尔值
local saved = nvs.get("enabled")
if saved ~= "" then
    enabled = (saved == "true")
end
```

---

##### `nvs.remove(key)`

删除键值对

**参数**：
- `key` (string): 键名

**返回值**：无

```lua
-- 删除键值对
nvs.remove("counter")

-- 重置所有数据
nvs.remove("weather_city")
nvs.remove("weather_temp")
nvs.remove("weather_updated")
```

---

#### NVS 完整示例

```lua
-- 元数据
CARD_NAME = "NVS 演示"
CARD_CATEGORY = "其他"
CARD_LOGO = "/icons/card_default.bin"
CARD_ORDER = 1
CARD_ENABLED = true

local counter = 0
local username = "Guest"

function onInit()
    -- 加载数据
    local savedCounter = nvs.get("demo_counter")
    if savedCounter ~= "" then
        counter = tonumber(savedCounter) or 0
    end

    local savedName = nvs.get("demo_username")
    if savedName ~= "" then
        username = savedName
    end

    sys.log("Loaded - Counter: " .. counter .. ", User: " .. username)
    _render()
end

function onExit()
    -- 保存数据
    nvs.set("demo_counter", tostring(counter))
    nvs.set("demo_username", username)
    sys.log("Saved - Counter: " .. counter .. ", User: " .. username)
end

function onLoop() end

function onBtnPress()
    counter = counter + 1
    _render()
end

function onBtnLong()
    -- 重置数据
    counter = 0
    username = "Guest"
    nvs.remove("demo_counter")
    nvs.remove("demo_username")
    _render()
end

function _render()
    eink.clear()
    eink.drawStr(10, 10, "User: " .. username)
    eink.drawStr(10, 30, "Counter: " .. counter)
    eink.drawStr(10, 50, "Press: +1")
    eink.drawStr(10, 65, "Long: Reset")

    if counter == 0 then
        eink.refreshDeep()
    else
        eink.refreshPartial()
    end
end
```

---

### 🌐 网络请求 API (`http.*`)

#### 安全限制

**HTTP 白名单机制**：只能访问以下域名

- `api.infinitytag.app`
- `192.168.*` (局域网)

访问其他域名会报错：`"URL not in whitelist"`

---

##### `http.get(url)`

发起 HTTP GET 请求

**参数**：
- `url` (string): 请求 URL（必须在白名单内）

**返回值**：`string` - 响应内容，失败时抛出错误

```lua
-- 基础 GET 请求
local success, response = pcall(function()
    return http.get("https://api.infinitytag.app/calendar/today")
end)

if success then
    sys.log("Response: " .. response)
else
    sys.log("Request failed: " .. tostring(response))
end

-- 局域网请求
local resp = http.get("http://192.168.1.100:8001/api/data")
```

**错误处理**：使用 `pcall()` 捕获错误

---

##### `http.post(url, body)`

发起 HTTP POST 请求

**参数**：
- `url` (string): 请求 URL
- `body` (string): 请求体（JSON 字符串）

**返回值**：`string` - 响应内容

**Content-Type**：自动设置为 `application/json`

```lua
-- POST 请求
local json = '{"name":"Alice","age":25}'
local success, response = pcall(function()
    return http.post("https://api.infinitytag.app/data", json)
end)

if success then
    sys.log("Response: " .. response)
else
    sys.log("POST failed: " .. tostring(response))
end
```

---

##### `http.downloadBitmap(url)`

下载位图数据并直接写入 framebuffer

**参数**：
- `url` (string): 位图文件 URL

**返回值**：
- `true`: 下载成功
- `false`: 下载失败或文件大小不匹配

**文件格式**：必须是 2808 字节的原始位图数据 (212×104÷8)

```lua
function onInit()
    sys.log("Downloading bitmap...")

    local success = http.downloadBitmap(
        "https://api.infinitytag.app/images/default.bin"
    )

    if success then
        sys.log("Bitmap downloaded")
        eink.refreshDeep()  -- 刷新显示图片
    else
        sys.log("Download failed")
        eink.clear()
        eink.drawStr(10, 40, "Download failed")
        eink.refresh()
    end
end
```

---

#### HTTP 完整示例：天气卡片

```lua
-- 元数据
CARD_NAME = "天气"
CARD_CATEGORY = "展示"
CARD_LOGO = "/icons/card_weather.bin"
CARD_ORDER = 10
CARD_ENABLED = true

local weather = "Loading..."
local temp = "--"

function onInit()
    _fetchWeather()
end

function onExit() end
function onLoop() end

function onBtnPress()
    _fetchWeather()
end

function _fetchWeather()
    eink.clear()
    eink.drawChinese(60, 20, "获取中...")
    eink.refresh()

    local success, response = pcall(function()
        return http.get("https://api.infinitytag.app/weather/current")
    end)

    if success then
        -- 简单的 JSON 解析（实际项目建议使用 JSON 库）
        -- 假设返回: {"weather":"晴","temp":"25"}
        weather = response:match('"weather":"([^"]+)"') or "Unknown"
        temp = response:match('"temp":"([^"]+)"') or "--"
        sys.log("Weather: " .. weather .. ", Temp: " .. temp)
    else
        weather = "Error"
        temp = "--"
        sys.log("Fetch failed: " .. tostring(response))
    end

    _render()
end

function _render()
    eink.clear()
    eink.drawChinese(60, 10, "今日天气")
    eink.drawChinese(30, 40, "天气: " .. weather)
    eink.drawChinese(30, 60, "温度: " .. temp .. "°C")
    eink.drawStr(30, 85, "Press to refresh")
    eink.refreshPartial()
end
```

---

### 🔧 硬件控制 API (`hw.*`)

**⚠️ 注意**：部分功能待实现，目前仅输出日志

---

##### `hw.vibrate(duration)`

触发震动

**参数**：
- `duration` (number): 震动时长（毫秒）

**返回值**：无

**状态**：⚠️ 待实现（目前仅输出日志）

```lua
-- 震动 100ms
hw.vibrate(100)

-- 震动 500ms
hw.vibrate(500)
```

---

##### `hw.sleep()`

进入深度睡眠

**参数**：无

**返回值**：无

**状态**：⚠️ 待实现（目前仅输出日志）

```lua
-- 进入睡眠
hw.sleep()
```

---

##### `hw.getBattery()`

获取电池电量

**参数**：无

**返回值**：`number` - 电量百分比 (0-100)

**状态**：⚠️ 临时返回 100%，待接入 ADC 读取

```lua
local battery = hw.getBattery()
sys.log("Battery: " .. battery .. "%")

eink.drawStr(10, 10, "Battery: " .. battery .. "%")
```

---

### 🛠️ 系统工具 API (`sys.*`)

##### `sys.log(message)`

输出日志到串口

**参数**：
- `message` (string): 日志内容

**返回值**：无

**输出格式**：`[Lua] <message>`

```lua
-- 输出日志
sys.log("Card initialized")

-- 输出变量
sys.log("Counter: " .. counter)

-- 调试信息
sys.log("Debug: x=" .. x .. ", y=" .. y)
```

**查看日志**：
```bash
pio device monitor
```

---

##### `sys.millis()`

获取系统运行时间

**参数**：无

**返回值**：`number` - 毫秒数（从启动开始）

```lua
-- 获取当前时间
local now = sys.millis()
sys.log("Uptime: " .. now .. "ms")

-- 实现定时器
local lastUpdate = 0

function onLoop()
    local now = sys.millis()
    if now - lastUpdate > 5000 then  -- 每 5 秒
        lastUpdate = now
        sys.log("5 seconds passed")
    end
end
```

---

##### `sys.delay(ms)`

延迟执行

**参数**：
- `ms` (number): 延迟时长（毫秒）

**返回值**：无

```lua
-- 延迟 1 秒
sys.delay(1000)

-- 延迟 500ms
sys.delay(500)
```

**⚠️ 警告**：
- 避免在 `onLoop()` 中使用，会阻塞主循环
- 建议使用 `sys.millis()` 实现非阻塞定时

```lua
-- ❌ 错误：阻塞主循环
function onLoop()
    sys.delay(1000)  -- 会阻塞整个系统
    _update()
end

-- ✅ 正确：非阻塞定时
local lastUpdate = 0

function onLoop()
    local now = sys.millis()
    if now - lastUpdate > 1000 then
        lastUpdate = now
        _update()
    end
end
```

---

### 📊 API 速查表

#### 墨水屏绘图 (`eink.*`)

| 函数 | 说明 | 示例 |
|------|------|------|
| `getWidth()` | 获取屏幕宽度 | `local w = eink.getWidth()` |
| `getHeight()` | 获取屏幕高度 | `local h = eink.getHeight()` |
| `clear()` | 清空屏幕 | `eink.clear()` |
| `drawPixel(x, y, color)` | 绘制像素 | `eink.drawPixel(50, 50, 0)` |
| `drawLine(x1, y1, x2, y2)` | 绘制直线 | `eink.drawLine(10, 10, 100, 50)` |
| `drawRect(x, y, w, h)` | 绘制矩形边框 | `eink.drawRect(10, 10, 100, 50)` |
| `fillRect(x, y, w, h)` | 绘制填充矩形 | `eink.fillRect(10, 10, 50, 30)` |
| `drawStr(x, y, text)` | 绘制 ASCII 文本 | `eink.drawStr(10, 10, "Hello")` |
| `drawChinese(x, y, text)` | 绘制中文文本 | `eink.drawChinese(10, 10, "你好")` |
| `refreshPartial()` | 局部刷新（快） | `eink.refreshPartial()` |
| `refresh()` | 全屏刷新（中） | `eink.refresh()` |
| `refreshDeep()` | 深度刷新（慢） | `eink.refreshDeep()` |

#### 永久存储 (`nvs.*`)

| 函数 | 说明 | 示例 |
|------|------|------|
| `set(key, value)` | 存储键值对 | `nvs.set("counter", "123")` |
| `get(key)` | 读取值 | `local v = nvs.get("counter")` |
| `remove(key)` | 删除键值对 | `nvs.remove("counter")` |

#### 网络请求 (`http.*`)

| 函数 | 说明 | 示例 |
|------|------|------|
| `get(url)` | HTTP GET 请求 | `local r = http.get(url)` |
| `post(url, body)` | HTTP POST 请求 | `local r = http.post(url, json)` |
| `downloadBitmap(url)` | 下载位图 | `http.downloadBitmap(url)` |

#### 硬件控制 (`hw.*`)

| 函数 | 说明 | 状态 | 示例 |
|------|------|------|------|
| `vibrate(duration)` | 触发震动 | ⚠️ 待实现 | `hw.vibrate(100)` |
| `sleep()` | 进入睡眠 | ⚠️ 待实现 | `hw.sleep()` |
| `getBattery()` | 获取电量 | ⚠️ 临时值 | `local b = hw.getBattery()` |

#### 系统工具 (`sys.*`)

| 函数 | 说明 | 示例 |
|------|------|------|
| `log(message)` | 输出日志 | `sys.log("Hello")` |
| `millis()` | 获取运行时间 | `local t = sys.millis()` |
| `delay(ms)` | 延迟执行 | `sys.delay(1000)` |

---

### 🎯 小结

**第三部分关键要点**：

1. **墨水屏 API**：12 个函数，支持基础绘图、文本、中文、三种刷新模式
2. **存储 API**：3 个函数，NVS 持久化存储（字符串类型）
3. **网络 API**：3 个函数，HTTP GET/POST/下载位图，白名单保护
4. **硬件 API**：3 个函数，部分待实现
5. **系统 API**：3 个函数，日志、时间、延迟

**接下来**：
- 查看 [第四部分：开发实战](#第四部分开发实战) 学习完整的示例项目

---

## 第四部分：开发实战

本部分提供 3 个完整的实战示例，从简单到复杂，帮助您掌握 Lua 卡片开发。

---

### 📊 示例 1：计数器卡片（基础）

**难度**：⭐ 基础

**功能**：
- 按键增加计数
- 长按重置计数
- 编码器调整计数
- 摇晃随机计数
- NVS 持久化存储

**完整代码**：

```lua
-- 元数据
CARD_NAME = "计数器"
CARD_CATEGORY = "其他"
CARD_LOGO = "/icons/card_default.bin"
CARD_ORDER = 10
CARD_ENABLED = true

-- 状态变量
local counter = 0
local lastUpdate = 0

-- 初始化
function onInit()
    sys.log("Counter card initialized")
    counter = 0

    -- 从 NVS 加载上次的计数
    local saved = nvs.get("counter_value")
    if saved and saved ~= "" then
        counter = tonumber(saved) or 0
        sys.log("Loaded counter from NVS: " .. counter)
    end

    _renderDeep()  -- 首次进入使用 DEEP 刷新
end

-- 退出
function onExit()
    sys.log("Counter card exited")

    -- 保存计数到 NVS
    nvs.set("counter_value", tostring(counter))
    sys.log("Saved counter to NVS: " .. counter)
end

-- 主循环
function onLoop()
    -- 可选：每 10 秒自动增加
    local now = sys.millis()
    if now - lastUpdate > 10000 then
        lastUpdate = now
        -- counter = counter + 1
        -- _render()
    end
end

-- 按键事件：增加计数
function onBtnPress()
    counter = counter + 1
    sys.log("Button pressed - counter: " .. counter)
    _render()
end

-- 长按事件：重置计数
function onBtnLong()
    counter = 0
    sys.log("Long press - counter reset")
    _render()
end

-- 编码器顺时针：增加 10
function onEncoderCW()
    counter = counter + 10
    sys.log("Encoder CW - counter: " .. counter)
    _render()
end

-- 编码器逆时针：减少 10
function onEncoderCCW()
    counter = counter - 10
    sys.log("Encoder CCW - counter: " .. counter)
    _render()
end

-- 摇晃事件：随机计数
function onShake()
    math.randomseed(sys.millis())
    counter = math.random(0, 100)
    sys.log("Device shaken - random counter: " .. counter)
    _render()
end

-- 渲染函数（DEEP 刷新）
function _renderDeep()
    sys.log("Rendering with DEEP refresh")
    eink.clear()

    -- 标题
    eink.drawChinese(60, 5, "计数器")

    -- 分割线
    eink.drawLine(0, 22, 211, 22)

    -- 计数器显示（大字）
    eink.drawStr(70, 40, tostring(counter))

    -- 操作说明
    eink.drawStr(10, 60, "Press: +1")
    eink.drawStr(10, 72, "Long: Reset")
    eink.drawStr(10, 84, "Rotate: +/-10")
    eink.drawStr(10, 96, "Shake: Random")

    eink.refreshDeep()
    sys.log("DEEP refresh completed")
end

-- 渲染函数（局部刷新）
function _render()
    eink.clear()

    -- 标题
    eink.drawChinese(60, 5, "计数器")

    -- 分割线
    eink.drawLine(0, 22, 211, 22)

    -- 计数器显示（大字）
    eink.drawStr(70, 40, tostring(counter))

    -- 操作说明
    eink.drawStr(10, 60, "Press: +1")
    eink.drawStr(10, 72, "Long: Reset")
    eink.drawStr(10, 84, "Rotate: +/-10")
    eink.drawStr(10, 96, "Shake: Random")

    eink.refreshPartial()
end
```

**学习要点**：
1. ✅ 使用 `local` 声明局部变量
2. ✅ NVS 持久化存储（`nvs.set/get`）
3. ✅ 首次进入使用 `refreshDeep()`，更新使用 `refreshPartial()`
4. ✅ 所有 6 种事件的处理
5. ✅ 使用 `sys.log()` 输出调试信息

---

### 🌤️ 示例 2：天气卡片（中级）

**难度**：⭐⭐ 中级

**功能**：
- HTTP GET 请求获取天气数据
- JSON 数据解析
- 错误处理（pcall）
- 按键刷新数据
- 加载状态显示

**完整代码**：

```lua
-- 元数据
CARD_NAME = "天气"
CARD_CATEGORY = "展示"
CARD_LOGO = "/icons/card_weather.bin"
CARD_ORDER = 20
CARD_ENABLED = true

-- 状态变量
local weather = "Unknown"
local temperature = "--"
local humidity = "--"
local city = "Beijing"
local lastFetch = 0
local isLoading = false

-- 初始化
function onInit()
    sys.log("Weather card initialized")

    -- 从 NVS 加载城市设置
    local savedCity = nvs.get("weather_city")
    if savedCity ~= "" then
        city = savedCity
    end

    -- 显示加载界面
    _renderLoading()

    -- 获取天气数据
    _fetchWeather()
end

-- 退出
function onExit()
    sys.log("Weather card exited")
    -- 保存城市设置
    nvs.set("weather_city", city)
end

-- 主循环
function onLoop()
    -- 每 30 分钟自动刷新
    local now = sys.millis()
    if now - lastFetch > 1800000 then  -- 30 分钟
        _fetchWeather()
    end
end

-- 按键事件：手动刷新
function onBtnPress()
    sys.log("Manual refresh requested")
    _renderLoading()
    _fetchWeather()
end

-- 长按事件：切换城市
function onBtnLong()
    sys.log("Switching city")
    if city == "Beijing" then
        city = "Shanghai"
    elseif city == "Shanghai" then
        city = "Guangzhou"
    else
        city = "Beijing"
    end

    nvs.set("weather_city", city)
    _renderLoading()
    _fetchWeather()
end

function onEncoderCW() end
function onEncoderCCW() end
function onShake() end

-- 获取天气数据
function _fetchWeather()
    if isLoading then
        sys.log("Already loading, skip")
        return
    end

    isLoading = true
    sys.log("Fetching weather for " .. city)

    -- 构建 API URL
    local apiUrl = "https://api.infinitytag.app/weather?city=" .. city

    -- 发起 HTTP GET 请求
    local success, response = pcall(function()
        return http.get(apiUrl)
    end)

    if success and response then
        sys.log("Weather data received: " .. response)

        -- 简单的 JSON 解析
        -- 假设返回格式: {"weather":"晴","temp":"25","humidity":"60"}
        weather = _parseJson(response, "weather") or "Unknown"
        temperature = _parseJson(response, "temp") or "--"
        humidity = _parseJson(response, "humidity") or "--"

        lastFetch = sys.millis()
        sys.log("Weather: " .. weather .. ", Temp: " .. temperature)
    else
        sys.log("Failed to fetch weather: " .. tostring(response))
        weather = "Error"
        temperature = "--"
        humidity = "--"
    end

    isLoading = false
    _render()
end

-- 简单的 JSON 解析函数
function _parseJson(json, key)
    local pattern = '"' .. key .. '":"([^"]+)"'
    local value = json:match(pattern)
    return value
end

-- 渲染加载界面
function _renderLoading()
    eink.clear()
    eink.drawChinese(60, 20, "加载中")
    eink.drawChinese(50, 45, city)
    eink.drawStr(60, 70, "Loading...")
    eink.refresh()
end

-- 渲染天气界面
function _render()
    eink.clear()

    -- 标题
    eink.drawChinese(70, 5, "天气")

    -- 城市
    eink.drawChinese(10, 25, "城市: " .. city)

    -- 天气状况
    eink.drawChinese(10, 45, "天气: " .. weather)

    -- 温度
    eink.drawChinese(10, 65, "温度: " .. temperature .. "°C")

    -- 湿度
    eink.drawChinese(10, 85, "湿度: " .. humidity .. "%")

    -- 操作提示
    eink.drawStr(10, 100, "Press: Refresh")

    eink.refreshPartial()
    sys.log("Weather display updated")
end
```

**学习要点**：
1. ✅ HTTP GET 请求（`http.get()`）
2. ✅ 错误处理（`pcall()`）
3. ✅ 简单的 JSON 解析（字符串匹配）
4. ✅ 加载状态显示
5. ✅ 定时自动刷新（30 分钟）
6. ✅ 防止重复请求（`isLoading` 标志）

**API 说明**：
- 示例使用的是假设的 API 端点
- 实际使用时需要替换为真实的天气 API
- 确保 API 域名在白名单内

---

### 🖼️ 示例 3：图片展示卡片（高级）

**难度**：⭐⭐⭐ 高级

**功能**：
- 下载并显示位图图片
- 图片加载状态显示
- 错误处理和重试
- 按键重新加载图片

**完整代码**：

```lua
-- 元数据
CARD_NAME = "图片"
CARD_CATEGORY = "展示"
CARD_LOGO = "/icons/card_image.bin"
CARD_ORDER = 30
CARD_ENABLED = true

-- 状态变量
local imageUrl = "https://api.infinitytag.app/images/default.bin"
local imageLoaded = false
local loadAttempts = 0
local maxAttempts = 3

-- 初始化
function onInit()
    sys.log("Image card initialized")
    imageLoaded = false
    loadAttempts = 0

    -- 从 NVS 加载图片 URL
    local savedUrl = nvs.get("image_url")
    if savedUrl ~= "" then
        imageUrl = savedUrl
        sys.log("Loaded image URL from NVS: " .. imageUrl)
    end

    -- 显示加载界面
    _renderLoading()

    -- 下载图片
    _loadImage()
end

-- 退出
function onExit()
    sys.log("Image card exited")
    -- 保存图片 URL
    nvs.set("image_url", imageUrl)
end

-- 主循环
function onLoop()
    -- 图片卡片不需要循环更新
end

-- 按键事件：重新加载图片
function onBtnPress()
    sys.log("Button pressed - reloading image")
    imageLoaded = false
    loadAttempts = 0
    _renderLoading()
    _loadImage()
end

-- 长按事件：切换图片
function onBtnLong()
    sys.log("Long press - switching image")

    -- 切换到不同的图片
    if imageUrl:find("default") then
        imageUrl = "https://api.infinitytag.app/images/image1.bin"
    elseif imageUrl:find("image1") then
        imageUrl = "https://api.infinitytag.app/images/image2.bin"
    else
        imageUrl = "https://api.infinitytag.app/images/default.bin"
    end

    nvs.set("image_url", imageUrl)
    imageLoaded = false
    loadAttempts = 0
    _renderLoading()
    _loadImage()
end

function onEncoderCW() end
function onEncoderCCW() end
function onShake() end

-- 加载图片
function _loadImage()
    if imageLoaded then
        sys.log("Image already loaded")
        return
    end

    if loadAttempts >= maxAttempts then
        sys.log("Max load attempts reached")
        _renderError("Max retries reached")
        return
    end

    loadAttempts = loadAttempts + 1
    sys.log("Loading image (attempt " .. loadAttempts .. "/" .. maxAttempts .. ")")
    sys.log("URL: " .. imageUrl)

    -- 下载位图
    local success = http.downloadBitmap(imageUrl)

    if success then
        imageLoaded = true
        sys.log("Image loaded successfully")

        -- 使用 DEEP 刷新显示图片
        eink.refreshDeep()

        -- 显示加载成功提示（3 秒后消失）
        sys.delay(3000)
    else
        sys.log("Failed to load image")

        if loadAttempts < maxAttempts then
            -- 重试
            _renderLoading("Retry " .. loadAttempts .. "/" .. maxAttempts)
            sys.delay(1000)
            _loadImage()
        else
            -- 达到最大重试次数
            _renderError("Failed after " .. maxAttempts .. " attempts")
        end
    end
end

-- 渲染加载界面
function _renderLoading(message)
    eink.clear()

    -- 标题
    eink.drawChinese(70, 20, "图片")

    -- 加载提示
    if message then
        eink.drawStr(50, 45, message)
    else
        eink.drawStr(50, 45, "Loading...")
    end

    -- 进度指示
    local barWidth = 100
    local barHeight = 10
    local barX = (212 - barWidth) / 2
    local barY = 60

    eink.drawRect(barX, barY, barWidth, barHeight)

    -- 填充进度
    local progress = loadAttempts / maxAttempts
    eink.fillRect(barX, barY, barWidth * progress, barHeight)

    eink.refresh()
end

-- 渲染错误界面
function _renderError(message)
    eink.clear()

    -- 标题
    eink.drawChinese(70, 20, "错误")

    -- 错误信息
    eink.drawStr(30, 45, message)

    -- 操作提示
    eink.drawStr(20, 70, "Press to retry")
    eink.drawStr(20, 85, "Long press to switch")

    eink.refresh()
end
```

**学习要点**：
1. ✅ 下载位图（`http.downloadBitmap()`）
2. ✅ 错误处理和重试机制
3. ✅ 进度条显示
4. ✅ 使用 `refreshDeep()` 显示图片
5. ✅ 状态管理（加载中、成功、失败）
6. ✅ URL 切换和持久化

**位图格式要求**：
- 文件大小：2808 字节
- 格式：原始位图数据（212×104÷8）
- 行对齐：27 字节/行 × 104 行
- 颜色：1-bit 黑白

**生成位图工具**：
```bash
# 使用 Python PIL 生成位图
python tools/generate_bitmap.py input.png output.bin
```

---

### 🎯 实战总结

| 示例 | 难度 | 核心技术 | 学习重点 |
|------|------|----------|----------|
| **计数器** | ⭐ 基础 | NVS 存储、事件处理 | 状态管理、持久化 |
| **天气** | ⭐⭐ 中级 | HTTP 请求、JSON 解析 | 网络请求、错误处理 |
| **图片** | ⭐⭐⭐ 高级 | 位图下载、重试机制 | 二进制数据、状态机 |

**开发建议**：
1. 从计数器示例开始，掌握基础
2. 学习天气示例，理解网络请求
3. 研究图片示例，处理复杂场景
4. 结合多个示例的技术，开发自己的卡片

**接下来**：
- 查看 [第五部分：部署和调试](#第五部分部署和调试) 学习如何部署和调试卡片

---

## 第五部分：部署和调试

### 📁 文件系统结构

Lua 卡片存储在 LittleFS 文件系统中，目录结构如下：

```
data/
├── cards/              # Lua 卡片目录
│   ├── template.lua    # 模板卡片
│   ├── image.lua       # 图片卡片
│   ├── calendar.lua    # 黄历卡片
│   └── weather.lua     # 天气卡片（自定义）
│
└── icons/              # 图标目录
    ├── card_default.bin    # 默认图标 (48x48)
    ├── card_weather.bin    # 天气图标
    ├── card_image.bin      # 图片图标
    └── card_calendar.bin   # 黄历图标
```

---

### 🚀 部署流程

#### 步骤 1：准备卡片文件

将您的 Lua 卡片放入 `data/cards/` 目录：

```bash
# Windows
copy my_card.lua infinity-tag-esp32\data\cards\

# Linux/Mac
cp my_card.lua infinity-tag-esp32/data/cards/
```

#### 步骤 2：配置 PlatformIO

确认 `platformio.ini` 中的文件系统配置：

```ini
[env:4d_systems_esp32s3_gen4_r8n16]
platform = espressif32
board = 4d_systems_esp32s3_gen4_r8n16
framework = arduino

# 文件系统配置
board_build.filesystem = littlefs
```

#### 步骤 3：上传文件系统

```bash
# 连接设备到 USB

# 上传文件系统（包含所有 Lua 卡片）
pio run -t uploadfs

# 等待上传完成
# 输出示例：
# Configuring upload protocol...
# AVAILABLE: cmsis-dap, esp-bridge, esp-builtin, esp-prog, espota, esptool, iot-bus-jtag, jlink, minimodule, olimex-arm-usb-ocd, olimex-arm-usb-ocd-h, olimex-arm-usb-tiny-h, olimex-jtag-tiny, tumpa
# CURRENT: upload_protocol = esptool
# Looking for upload port...
# Auto-detected: COM3
# Uploading .pio/build/4d_systems_esp32s3_gen4_r8n16/littlefs.bin
# esptool.py v4.5.1
# Serial port COM3
# Connecting....
# Chip is ESP32-S3 (revision v0.1)
# Features: WiFi, BLE
# Crystal is 40MHz
# MAC: 34:85:18:xx:xx:xx
# Uploading stub...
# Running stub...
# Stub running...
# Configuring flash size...
# Flash will be erased from 0x00110000 to 0x0020ffff...
# Compressed 1048576 bytes to 12345...
# Writing at 0x00110000... (100 %)
# Wrote 1048576 bytes (12345 compressed) at 0x00110000 in 2.3 seconds (effective 3.6 Mbit/s)...
# Hash of data verified.
#
# Leaving...
# Hard resetting via RTS pin...
```

#### 步骤 4：重启设备

上传完成后，设备会自动重启并加载新的 Lua 卡片。

---

### 🔍 调试方法

#### 串口调试

**打开串口监视器**：

```bash
pio device monitor

# 或指定波特率
pio device monitor -b 115200
```

**查看日志输出**：

```
[System] Infinity Tag ESP32 v1.0.0.9
[System] Free heap: 245760 bytes
[LittleFS] Mounting filesystem...
[LittleFS] Filesystem mounted

[LuaCard] Loading cards from /cards
[LuaCard] Found: template.lua
[LuaCard] Found: image.lua
[LuaCard] Found: weather.lua
[LuaCard] Loaded 3 Lua cards

[CardManager] Registered 8 cards total
[System] Ready!

[Lua] Template card initialized
[Lua] Loaded counter from NVS: 42
[Lua] _renderDeep() started
[Lua] eink.clear() done
[Lua] Title drawn
[Lua] Counter drawn
[Lua] Instructions drawn
[Lua] About to call eink.refreshDeep()
[Lua] eink.refreshDeep() completed
```

---

#### 使用 `sys.log()` 调试

在代码中添加日志输出：

```lua
function onInit()
    sys.log("=== onInit() START ===")
    sys.log("Counter: " .. counter)

    local saved = nvs.get("my_key")
    sys.log("NVS value: " .. saved)

    if saved ~= "" then
        counter = tonumber(saved) or 0
        sys.log("Loaded counter: " .. counter)
    else
        sys.log("No saved value found")
    end

    sys.log("=== onInit() END ===")
    _render()
end
```

**输出示例**：

```
[Lua] === onInit() START ===
[Lua] Counter: 0
[Lua] NVS value: 42
[Lua] Loaded counter: 42
[Lua] === onInit() END ===
```

---

#### 调试技巧

**1. 分段调试**

```lua
function _complexFunction()
    sys.log("Step 1: Start")
    local data = _fetchData()
    sys.log("Step 2: Data fetched, length: " .. #data)

    local parsed = _parseData(data)
    sys.log("Step 3: Data parsed, count: " .. #parsed)

    _renderData(parsed)
    sys.log("Step 4: Render complete")
end
```

**2. 变量检查**

```lua
function onBtnPress()
    sys.log("Button pressed")
    sys.log("counter = " .. tostring(counter))
    sys.log("mode = " .. tostring(mode))
    sys.log("enabled = " .. tostring(enabled))

    counter = counter + 1
    _render()
end
```

**3. 错误捕获**

```lua
function _safeOperation()
    local success, result = pcall(function()
        -- 可能出错的操作
        return http.get(url)
    end)

    if success then
        sys.log("Operation succeeded: " .. result)
        return result
    else
        sys.log("Operation failed: " .. tostring(result))
        return nil
    end
end
```

---

### 🐛 错误卡片显示

当 Lua 卡片加载失败时，系统会显示错误卡片：

**错误类型**：

1. **语法错误**
   ```
   [LuaCard] Syntax error in weather.lua:
   Line 42: unexpected symbol near '}'
   ```

2. **运行时错误**
   ```
   [Lua] Runtime error in onInit():
   attempt to call a nil value (field 'drawString')
   ```

3. **超时错误**
   ```
   [Lua] Script execution timeout (5000 ms)
   Possible infinite loop in onLoop()
   ```

**错误卡片界面**：

```
┌──────────────────────┐
│   ⚠️ Lua Error       │
├──────────────────────┤
│ Card: weather.lua    │
│                      │
│ Syntax error:        │
│ Line 42              │
│ unexpected symbol    │
│                      │
│ Check serial log     │
│ for details          │
└──────────────────────┘
```

---

### ⚡ 性能优化

#### 1. 刷新策略优化

```lua
-- ❌ 错误：频繁使用 DEEP 刷新
function onBtnPress()
    counter = counter + 1
    eink.clear()
    eink.drawStr(10, 10, "Count: " .. counter)
    eink.refreshDeep()  -- 太慢！每次 900ms
end

-- ✅ 正确：首次 DEEP，更新 Partial
local isFirstRender = true

function _render()
    eink.clear()
    eink.drawStr(10, 10, "Count: " .. counter)

    if isFirstRender then
        eink.refreshDeep()
        isFirstRender = false
    else
        eink.refreshPartial()  -- 快速更新
    end
end
```

#### 2. 避免阻塞操作

```lua
-- ❌ 错误：在 onLoop() 中使用 delay
function onLoop()
    sys.delay(1000)  -- 阻塞整个系统！
    _update()
end

-- ✅ 正确：使用 millis() 实现非阻塞定时
local lastUpdate = 0

function onLoop()
    local now = sys.millis()
    if now - lastUpdate > 1000 then
        lastUpdate = now
        _update()
    end
end
```

#### 3. 减少不必要的刷新

```lua
-- ❌ 错误：每次都刷新
function onEncoderCW()
    value = value + 1
    _render()  -- 旋转编码器时频繁刷新
end

-- ✅ 正确：累积变化后再刷新
local value = 0
local pendingUpdate = false

function onEncoderCW()
    value = value + 1
    pendingUpdate = true
end

function onLoop()
    if pendingUpdate then
        _render()
        pendingUpdate = false
    end
end
```

#### 4. 优化字符串操作

```lua
-- ❌ 错误：频繁字符串拼接
function _render()
    local text = ""
    text = text .. "Line 1\n"
    text = text .. "Line 2\n"
    text = text .. "Line 3\n"
    -- 多次内存分配
end

-- ✅ 正确：直接绘制
function _render()
    eink.drawStr(10, 10, "Line 1")
    eink.drawStr(10, 25, "Line 2")
    eink.drawStr(10, 40, "Line 3")
end
```

---

### 📊 内存管理

#### 查看内存使用

Lua 虚拟机会自动管理内存，但可以通过日志监控：

```
[System] Free heap: 245760 bytes
[System] PSRAM free: 8388608 bytes
[Lua] Memory usage: 65536 bytes
```

#### 内存优化建议

1. **使用局部变量**
   ```lua
   -- ✅ 正确
   local counter = 0
   local data = {}

   -- ❌ 错误
   counter = 0  -- 全局变量
   data = {}
   ```

2. **及时释放大对象**
   ```lua
   function _processLargeData()
       local largeData = http.get(url)
       local result = _parse(largeData)
       largeData = nil  -- 释放内存
       return result
   end
   ```

3. **避免内存泄漏**
   ```lua
   -- ❌ 错误：循环引用
   local a = {}
   local b = {}
   a.ref = b
   b.ref = a  -- 循环引用

   -- ✅ 正确：避免循环引用
   local a = {}
   local b = {}
   a.ref = b
   -- b 不引用 a
   ```

---

### 🎯 小结

**第五部分关键要点**：

1. **文件系统**：LittleFS，`data/cards/` 目录
2. **部署命令**：`pio run -t uploadfs`
3. **调试方法**：串口监视器 + `sys.log()`
4. **错误处理**：错误卡片显示，查看串口日志
5. **性能优化**：刷新策略、非阻塞定时、减少刷新
6. **内存管理**：使用局部变量、及时释放、避免泄漏

**接下来**：
- 查看 [第六部分：最佳实践](#第六部分最佳实践) 学习代码规范和安全考虑

---

## 第六部分：最佳实践

### 📝 代码规范

#### 命名约定

```lua
-- ✅ 正确的命名
local counter = 0              -- 局部变量：小写+下划线
local maxRetries = 3           -- 驼峰命名
local API_URL = "https://..."  -- 常量：全大写+下划线

CARD_NAME = "天气"             -- 元数据：全大写+下划线

function onInit() end          -- 回调函数：驼峰命名
function _render() end         -- 私有函数：下划线前缀

-- ❌ 错误的命名
Counter = 0                    -- 不要使用全局变量
my-variable = 0                -- 不要使用连字符
function OnInit() end          -- 回调函数首字母小写
```

#### 注释风格

```lua
-- 单行注释：简短说明

--[[
多行注释：
详细说明复杂逻辑
]]

-- 元数据
CARD_NAME = "天气"
CARD_CATEGORY = "展示"
CARD_LOGO = "/icons/card_weather.bin"
CARD_ORDER = 10
CARD_ENABLED = true

-- 状态变量
local temperature = 0    -- 温度（摄氏度）
local humidity = 0       -- 湿度（百分比）
local lastUpdate = 0     -- 上次更新时间（毫秒）

-- 初始化函数
-- 从 NVS 加载数据并显示初始界面
function onInit()
    sys.log("Weather card initialized")

    -- 加载保存的数据
    _loadFromNVS()

    -- 获取最新天气
    _fetchWeather()
end
```

#### 代码组织

```lua
-- ============================================
-- 1. 元数据定义
-- ============================================
CARD_NAME = "天气"
CARD_CATEGORY = "展示"
CARD_LOGO = "/icons/card_weather.bin"
CARD_ORDER = 10
CARD_ENABLED = true

-- ============================================
-- 2. 常量定义
-- ============================================
local API_URL = "https://api.infinitytag.app/weather"
local UPDATE_INTERVAL = 1800000  -- 30 分钟
local MAX_RETRIES = 3

-- ============================================
-- 3. 状态变量
-- ====================================
local temperature = 0
local humidity = 0
local weather = "Unknown"
local lastUpdate = 0

-- ============================================
-- 4. 生命周期函数
-- ============================================
function onInit()
    -- 初始化逻辑
end

function onExit()
    -- 清理逻辑
end

function onLoop()
    -- 循环逻辑
end

-- ============================================
-- 5. 事件处理函数
-- ============================================
function onBtnPress()
    -- 按键处理
end

function onBtnLong()
    -- 长按处理
end

-- ============================================
-- 6. 私有辅助函数
-- ============================================
function _fetchWeather()
    -- 获取天气数据
end

function _render()
    -- 渲染界面
end

function _loadFromNVS()
    -- 从 NVS 加载
end
```

---

### 🔒 安全考虑

#### 1. HTTP 白名单

**限制**：只能访问白名单域名
- `api.infinitytag.app`
- `192.168.*` (局域网)

```lua
-- ✅ 允许
http.get("https://api.infinitytag.app/weather")
http.get("http://192.168.1.100:8001/api/data")

-- ❌ 禁止（会报错）
http.get("https://evil.com/malware")
http.get("https://google.com")
```

**原因**：防止恶意脚本访问敏感 API 或泄露数据

---

#### 2. 超时保护

**限制**：脚本执行超过 5 秒会被强制终止

```lua
-- ❌ 错误：死循环
function onInit()
    while true do
        -- 永远不会结束
    end
end
-- 结果：5 秒后报错 "Script execution timeout"

-- ✅ 正确：使用有限循环
function onInit()
    for i = 1, 100 do
        -- 有限次数
    end
end
```

**原因**：防止死循环导致系统卡死

---

#### 3. 沙箱机制

**禁用的 Lua 函数**：
- `dofile()` - 防止加载任意文件
- `loadfile()` - 防止加载任意文件
- `require()` - 防止加载模块
- `package.*` - 防止包管理

```lua
-- ❌ 禁止（会报错）
dofile("/etc/passwd")
require("socket")
package.loadlib("evil.so")

-- ✅ 允许
local data = http.get(url)
local value = nvs.get("key")
```

**原因**：防止恶意脚本访问系统资源

---

#### 4. NVS 命名空间隔离

**注意**：所有 Lua 卡片共享同一个 NVS 命名空间

```lua
-- ❌ 错误：使用通用键名（可能冲突）
nvs.set("counter", "123")
nvs.set("data", "abc")

-- ✅ 正确：使用唯一键名
nvs.set("weather_counter", "123")
nvs.set("weather_data", "abc")
```

**建议**：使用 `<卡片名>_<键名>` 格式

---

### 👤 用户体验

#### 1. 刷新策略

```lua
-- ✅ 最佳实践
function onInit()
    -- 首次进入：使用 DEEP 刷新，彻底清除残影
    eink.refreshDeep()
end

function onBtnPress()
    -- 频繁更新：使用 Partial 刷新，速度快
    eink.refreshPartial()
end

function onShake()
    -- 重要更新：使用 Full 刷新，平衡速度和质量
    eink.refresh()
end
```

#### 2. 加载状态显示

```lua
-- ✅ 正确：显示加载状态
function _fetchData()
    -- 显示加载界面
    eink.clear()
    eink.drawChinese(60, 40, "加载中...")
    eink.refresh()

    -- 获取数据
    local data = http.get(url)

    -- 显示结果
    _render(data)
end

-- ❌ 错误：没有加载提示
function _fetchData()
    -- 用户不知道发生了什么
    local data = http.get(url)
    _render(data)
end
```

#### 3. 错误提示

```lua
-- ✅ 正确：友好的错误提示
function _fetchWeather()
    local success, response = pcall(function()
        return http.get(url)
    end)

    if not success then
        eink.clear()
        eink.drawChinese(50, 30, "网络错误")
        eink.drawStr(30, 50, "Press to retry")
        eink.refresh()
        return
    end

    _render(response)
end

-- ❌ 错误：没有错误处理
function _fetchWeather()
    local response = http.get(url)  -- 可能失败
    _render(response)
end
```

---

### ⚠️ 常见陷阱

#### 1. 死循环

```lua
-- ❌ 错误：死循环
function onLoop()
    while true do
        -- 永远不会结束
    end
end
-- 结果：5 秒后超时

-- ✅ 正确：使用定时器
local lastUpdate = 0

function onLoop()
    local now = sys.millis()
    if now - lastUpdate > 1000 then
        lastUpdate = now
        _update()
    end
end
```

#### 2. 内存泄漏

```lua
-- ❌ 错误：全局变量累积
function onBtnPress()
    data = http.get(url)  -- 全局变量，永不释放
    _process(data)
end

-- ✅ 正确：使用局部变量
function onBtnPress()
    local data = http.get(url)  -- 局部变量，自动释放
    _process(data)
end
```

#### 3. 阻塞操作

```lua
-- ❌ 错误：在 onLoop() 中使用 delay
function onLoop()
    sys.delay(1000)  -- 阻塞整个系统
    _update()
end

-- ✅ 正确：非阻塞定时
local lastUpdate = 0

function onLoop()
    local now = sys.millis()
    if now - lastUpdate > 1000 then
        lastUpdate = now
        _update()
    end
end
```

#### 4. 字符串类型错误

```lua
-- ❌ 错误：NVS 存储数字
nvs.set("counter", 123)  -- 错误！只能存储字符串

-- ✅ 正确：转换为字符串
nvs.set("counter", tostring(123))

-- 读取时转换回数字
local saved = nvs.get("counter")
if saved ~= "" then
    counter = tonumber(saved) or 0
end
```

#### 5. 忘记刷新屏幕

```lua
-- ❌ 错误：绘制后没有刷新
function onBtnPress()
    eink.clear()
    eink.drawStr(10, 10, "Hello")
    -- 忘记调用 refresh()，屏幕不会更新
end

-- ✅ 正确：绘制后刷新
function onBtnPress()
    eink.clear()
    eink.drawStr(10, 10, "Hello")
    eink.refreshPartial()  -- 刷新屏幕
end
```

---

### 🎯 小结

**第六部分关键要点**：

1. **代码规范**：命名约定、注释风格、代码组织
2. **安全机制**：HTTP 白名单、超时保护、沙箱、NVS 隔离
3. **用户体验**：刷新策略、加载状态、错误提示
4. **常见陷阱**：死循环、内存泄漏、阻塞操作、类型错误

**接下来**：
- 查看 [第七部分：故障排查](#第七部分故障排查) 解决常见问题

---

## 第七部分：故障排查

### 🐛 常见错误

#### 错误 1：语法错误

**症状**：
```
[LuaCard] Syntax error in weather.lua:
Line 42: unexpected symbol near '}'
```

**原因**：Lua 语法错误

**解决方案**：
1. 检查第 42 行附近的代码
2. 常见问题：
   - 缺少 `end` 关键字
   - 多余的逗号或括号
   - 字符串引号不匹配

```lua
-- ❌ 错误
function onInit()
    if counter > 0 then
        _render()
    -- 缺少 end
end

-- ✅ 正确
function onInit()
    if counter > 0 then
        _render()
    end  -- 添加 end
end
```

---

#### 错误 2：超时错误

**症状**：
```
[Lua] Script execution timeout (5000 ms)
Possible infinite loop in onLoop()
```

**原因**：脚本执行超过 5 秒

**解决方案**：
1. 检查是否有死循环
2. 检查是否有阻塞操作

```lua
-- ❌ 错误：死循环
function onInit()
    while true do
        -- 永远不会结束
    end
end

-- ✅ 正确：有限循环
function onInit()
    for i = 1, 100 do
        -- 有限次数
    end
end

-- ❌ 错误：阻塞操作
function onLoop()
    sys.delay(10000)  -- 10 秒延迟
end

-- ✅ 正确：非阻塞
local lastUpdate = 0
function onLoop()
    local now = sys.millis()
    if now - lastUpdate > 10000 then
        lastUpdate = now
        _update()
    end
end
```

---

#### 错误 3：HTTP 白名单错误

**症状**：
```
[Lua] HTTP request failed: URL not in whitelist
```

**原因**：访问的域名不在白名单内

**解决方案**：
1. 检查 URL 是否正确
2. 确保域名在白名单内：
   - `api.infinitytag.app`
   - `192.168.*`

```lua
-- ❌ 错误：不在白名单
http.get("https://google.com")

-- ✅ 正确：在白名单内
http.get("https://api.infinitytag.app/weather")
http.get("http://192.168.1.100:8001/api/data")
```

---

#### 错误 4：NVS 类型错误

**症状**：
```
[Lua] Runtime error: attempt to perform arithmetic on a string value
```

**原因**：从 NVS 读取的值是字符串，未转换为数字

**解决方案**：
使用 `tonumber()` 转换

```lua
-- ❌ 错误：直接使用字符串
local saved = nvs.get("counter")
counter = saved + 1  -- 错误！saved 是字符串

-- ✅ 正确：转换为数字
local saved = nvs.get("counter")
if saved ~= "" then
    counter = tonumber(saved) or 0
end
counter = counter + 1
```

---

#### 错误 5：函数未定义

**症状**：
```
[Lua] Runtime error: attempt to call a nil value (field 'drawString')
```

**原因**：调用了不存在的函数

**解决方案**：
检查函数名是否正确

```lua
-- ❌ 错误：函数名错误
eink.drawString(10, 10, "Hello")  -- 不存在

-- ✅ 正确：使用正确的函数名
eink.drawStr(10, 10, "Hello")     -- ASCII 文本
eink.drawChinese(10, 10, "你好")  -- 中文文本
```

---

#### 错误 6：屏幕不更新

**症状**：绘制了内容但屏幕没有变化

**原因**：忘记调用刷新函数

**解决方案**：
绘制后调用 `refresh()` 系列函数

```lua
-- ❌ 错误：没有刷新
function onBtnPress()
    eink.clear()
    eink.drawStr(10, 10, "Hello")
    -- 屏幕不会更新
end

-- ✅ 正确：添加刷新
function onBtnPress()
    eink.clear()
    eink.drawStr(10, 10, "Hello")
    eink.refreshPartial()  -- 刷新屏幕
end
```

---

### 🔍 调试技巧

#### 1. 使用 `sys.log()` 追踪执行流程

```lua
function onInit()
    sys.log("=== onInit START ===")

    sys.log("Step 1: Loading from NVS")
    local saved = nvs.get("counter")
    sys.log("Saved value: " .. saved)

    sys.log("Step 2: Parsing value")
    counter = tonumber(saved) or 0
    sys.log("Counter: " .. counter)

    sys.log("Step 3: Rendering")
    _render()

    sys.log("=== onInit END ===")
end
```

#### 2. 检查变量类型

```lua
function _debug()
    sys.log("counter type: " .. type(counter))
    sys.log("counter value: " .. tostring(counter))

    sys.log("weather type: " .. type(weather))
    sys.log("weather value: " .. weather)
end
```

#### 3. 捕获错误

```lua
function _safeCall()
    local success, result = pcall(function()
        -- 可能出错的代码
        return http.get(url)
    end)

    if success then
        sys.log("Success: " .. result)
        return result
    else
        sys.log("Error: " .. tostring(result))
        return nil
    end
end
```

---

### ❓ FAQ

#### Q1: 如何查看卡片是否加载成功？

**A**: 查看串口日志

```bash
pio device monitor

# 查找类似输出：
# [LuaCard] Found: weather.lua
# [LuaCard] Loaded 3 Lua cards
```

---

#### Q2: 为什么我的卡片没有出现在列表中？

**A**: 检查以下几点：
1. 文件是否在 `data/cards/` 目录
2. 文件扩展名是否为 `.lua`
3. `CARD_ENABLED` 是否为 `true`
4. 是否执行了 `pio run -t uploadfs`
5. 设备是否重启

---

#### Q3: 如何修改已上传的卡片？

**A**: 修改后重新上传文件系统

```bash
# 1. 修改 Lua 文件
# 2. 重新上传
pio run -t uploadfs
# 3. 设备会自动重启
```

---

#### Q4: 为什么 HTTP 请求总是失败？

**A**: 检查以下几点：
1. URL 是否在白名单内
2. 设备是否连接到 WiFi
3. 后端服务是否正常运行
4. 使用 `pcall()` 捕获错误信息

```lua
local success, response = pcall(function()
    return http.get(url)
end)

if not success then
    sys.log("HTTP error: " .. tostring(response))
end
```

---

#### Q5: 如何清除 NVS 中的数据？

**A**: 使用 `nvs.remove()`

```lua
-- 清除单个键
nvs.remove("weather_counter")

-- 清除所有相关键
nvs.remove("weather_city")
nvs.remove("weather_temp")
nvs.remove("weather_updated")
```

或者在设置菜单中选择"恢复出厂"。

---

#### Q6: 为什么屏幕有残影？

**A**: 使用深度刷新

```lua
-- 首次进入或需要彻底清除残影时
eink.refreshDeep()

-- 或者每 5 次局部刷新后使用一次全屏刷新
local refreshCount = 0

function _render()
    eink.clear()
    eink.drawStr(10, 10, "Hello")

    refreshCount = refreshCount + 1
    if refreshCount >= 5 then
        eink.refresh()  -- 全屏刷新
        refreshCount = 0
    else
        eink.refreshPartial()  -- 局部刷新
    end
end
```

---

#### Q7: 如何实现定时更新？

**A**: 使用 `sys.millis()` 在 `onLoop()` 中实现

```lua
local lastUpdate = 0
local UPDATE_INTERVAL = 60000  -- 1 分钟

function onLoop()
    local now = sys.millis()
    if now - lastUpdate > UPDATE_INTERVAL then
        lastUpdate = now
        _update()
    end
end
```

---

#### Q8: 如何调试网络请求？

**A**: 使用 `sys.log()` 输出详细信息

```lua
function _fetchData()
    sys.log("Fetching data from: " .. url)

    local success, response = pcall(function()
        return http.get(url)
    end)

    if success then
        sys.log("Response length: " .. #response)
        sys.log("Response: " .. response)
    else
        sys.log("Request failed: " .. tostring(response))
    end
end
```

---

### 🎯 小结

**第七部分关键要点**：

1. **常见错误**：语法错误、超时、白名单、类型错误、函数未定义、屏幕不更新
2. **调试技巧**：`sys.log()` 追踪、类型检查、错误捕获
3. **FAQ**：8 个常见问题和解决方案

---

## 🎉 恭喜！

您已经完成了 Lua 卡片完整开发指南的学习！

**您现在可以**：
- ✅ 编写自己的 Lua 卡片
- ✅ 使用所有 25+ 个 API 函数
- ✅ 部署和调试卡片
- ✅ 遵循最佳实践
- ✅ 解决常见问题

**下一步**：
1. 参考 [示例代码](#第四部分开发实战) 开始开发
2. 查阅 [API 参考](#第三部分api-参考手册) 了解详细用法
3. 遇到问题查看 [故障排查](#第七部分故障排查)

---

## 📚 附录

### 相关文档

- [项目 README](../README.md) - 项目概况
- [ARCHITECTURE.md](./ARCHITECTURE.md) - 系统架构
- [CONTRIBUTING.md](./CONTRIBUTING.md) - 开发指南
- [DEPLOYMENT.md](./DEPLOYMENT.md) - 部署运维

### 外部资源

- [Lua 5.3 参考手册](https://www.lua.org/manual/5.3/)
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/)
- [GxEPD2 库文档](https://github.com/ZinggJM/GxEPD2)

### 获取帮助

- **GitHub Issues** - 报告 Bug 和功能请求
- **GitHub Discussions** - 技术讨论和问答
- **串口日志** - 查看详细的运行日志

---

**文档版本**: v1.0
**最后更新**: 2026-02-08
**维护者**: Infinity Tag Team
**文档字数**: 约 18,000 字

---

💡 **感谢您使用 Infinity Tag ESP32 Lua 卡片系统！**
