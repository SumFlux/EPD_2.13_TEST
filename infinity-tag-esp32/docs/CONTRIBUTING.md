# 🤝 贡献指南

欢迎为 Infinity Tag ESP32 项目做出贡献！本文档将帮助您快速上手开发。

---

## 📋 目录

- [开发环境设置](#开发环境设置)
- [项目结构](#项目结构)
- [开发工作流](#开发工作流)
- [编译和上传](#编译和上传)
- [测试流程](#测试流程)
- [代码规范](#代码规范)
- [提交规范](#提交规范)
- [常用工具](#常用工具)

---

## 🛠️ 开发环境设置

### 必需工具

| 工具 | 版本 | 用途 |
|------|------|------|
| **PlatformIO** | 最新版 | 编译、上传、调试 |
| **Python** | 3.8+ | 字体转换、图标生成 |
| **Git** | 2.0+ | 版本控制 |
| **VSCode** | 推荐 | IDE（可选） |

### 安装步骤

```bash
# 1. 安装 PlatformIO
pip install platformio

# 2. 克隆项目
git clone <repository-url>
cd infinity-tag-esp32

# 3. 安装依赖库
pio lib install

# 4. 安装 Python 依赖（用于工具脚本）
pip install pillow
```

### VSCode 配置

安装推荐扩展：
- **PlatformIO IDE** - 核心开发工具
- **C/C++** - 代码智能提示
- **GitLens** - Git 增强

---

## 📁 项目结构

```
infinity-tag-esp32/
├── include/                 # 头文件
│   ├── Cards/              # 卡片类头文件
│   ├── Core/               # 核心系统（事件、卡片管理）
│   ├── Driver/             # 硬件驱动（EPD、输入）
│   ├── Fonts/              # 字体数据
│   ├── Network/            # 网络功能（WiFi、OTA）
│   ├── Utils/              # 工具类（日志、字体渲染）
│   └── Version.h           # 版本号定义
│
├── src/                    # 源文件
│   ├── Cards/              # 卡片实现
│   ├── Core/               # 核心系统实现
│   ├── Driver/             # 驱动实现
│   ├── Network/            # 网络功能实现
│   └── main.cpp            # 主程序入口
│
├── lib/                    # 第三方库（如 Lua）
├── data/                   # 文件系统数据（图标、脚本）
├── tools/                  # 开发工具脚本
│   ├── convert_font.py     # 字体转换工具
│   └── generate_icons.py   # 图标生成工具
│
├── docs/                   # 文档
├── platformio.ini          # PlatformIO 配置
└── README.md               # 项目说明
```

---

## 🔄 开发工作流

### 1. 创建功能分支

```bash
# 从 master 创建新分支
git checkout -b feature/your-feature-name

# 或修复 bug
git checkout -b fix/bug-description
```

### 2. 开发和测试

```bash
# 编译项目
pio run

# 上传到设备
pio run -t upload

# 查看串口输出
pio device monitor

# 或者一步完成：编译+上传+监控
pio run -t upload && pio device monitor
```

### 3. 代码审查

在提交前运行代码审查：

```bash
# 使用 Claude Code 进行代码审查
/everything-claude-code:code-review
```

### 4. 提交更改

```bash
# 添加更改
git add .

# 提交（遵循提交规范）
git commit -m "feat(cards): 添加新的天气卡片"

# 推送到远程
git push origin feature/your-feature-name
```

### 5. 创建 Pull Request

- 在 GitHub 上创建 PR
- 填写 PR 模板
- 等待代码审查
- 根据反馈修改代码

---

## 🔨 编译和上传

### PlatformIO 命令

| 命令 | 说明 |
|------|------|
| `pio run` | 编译项目 |
| `pio run -t upload` | 上传固件到设备 |
| `pio run -t uploadfs` | 上传文件系统（LittleFS） |
| `pio run -t clean` | 清理编译缓存 |
| `pio device monitor` | 打开串口监视器 |
| `pio device list` | 列出可用串口 |
| `pio lib list` | 列出已安装的库 |
| `pio lib update` | 更新依赖库 |

### 编译选项

在 `platformio.ini` 中配置：

```ini
[env:4d_systems_esp32s3_gen4_r8n16]
platform = espressif32
board = 4d_systems_esp32s3_gen4_r8n16
framework = arduino

# 依赖库
lib_deps =
    zinggjm/GxEPD2@^1.6.6
    adafruit/Adafruit GFX Library@^1.12.1
    bblanchon/ArduinoJson@^7.0.0
    https://github.com/tzapu/WiFiManager.git
    ricmoo/QRCode@^0.0.1
    olikraus/U8g2_for_Adafruit_GFX@^1.8.0

# 编译标志
build_flags =
    -DBOARD_HAS_PSRAM              # 启用 PSRAM
    -DARDUINO_USB_CDC_ON_BOOT=1    # USB CDC 启动
    -DCORE_DEBUG_LEVEL=3           # 调试级别
    -DLUA_USE_C89                  # Lua 配置
    -DLUA_USE_LONGJMP
    -DLUA_COMPAT_5_3=0
    -DLUA_COMPAT_LOADLIB=0
    -DLUA_USE_PSRAM

# 分区表
board_build.partitions = default_16MB.csv
board_build.filesystem = littlefs
```

### 调试日志控制

在 `include/Utils/Logger.h` 中：

```cpp
// 启用调试日志
#define ENABLE_DEBUG_LOGGING

// 禁用调试日志（发布版本）
// #define ENABLE_DEBUG_LOGGING
```

---

## 🧪 测试流程

### 单元测试

目前项目主要依赖硬件测试，未来计划添加：
- [ ] 核心逻辑单元测试
- [ ] 事件系统测试
- [ ] 卡片管理器测试

### 硬件测试清单

在提交前，请确保以下功能正常：

#### 基础功能
- [ ] 设备正常启动，显示启动画面
- [ ] WiFi 配置门户可以正常打开
- [ ] 连接 WiFi 后显示 IP 地址
- [ ] 串口输出正常，无异常错误

#### 交互功能
- [ ] 旋转编码器可以切换卡片
- [ ] 单击按键可以确认选择
- [ ] 长按 2 秒进入设置菜单
- [ ] 三击触发特殊功能
- [ ] 振动开关可以触发事件

#### 显示功能
- [ ] 局部刷新正常，无残影
- [ ] 全屏刷新正常
- [ ] 中文字体显示正确
- [ ] 图标显示正确
- [ ] 状态栏信息正确

#### 网络功能
- [ ] 可以从后端获取图片
- [ ] OTA 更新功能正常
- [ ] JWT 认证正常

#### 内存和性能
- [ ] 运行 1 小时无崩溃
- [ ] 内存使用稳定，无泄漏
- [ ] 响应速度正常

### 测试工具

```bash
# 查看内存使用
# 在串口输出中查找 "Free heap" 信息

# 查看任务栈使用
# 在串口输出中查找 "Stack high water mark" 信息

# 性能分析
# 使用 LOG_PRINTF 记录关键操作的耗时
```

---

## 📝 代码规范

### C++ 代码风格

#### 命名规范

```cpp
// 类名：大驼峰
class CardManager { };

// 函数名：小驼峰
void renderCard() { }

// 成员变量：下划线前缀
int _selectedIndex;

// 常量：全大写+下划线
const int MAX_CARDS = 10;

// 宏定义：全大写+下划线
#define PIN_CS 10
```

#### 文件组织

```cpp
// 1. 头文件保护
#ifndef CARD_MANAGER_H
#define CARD_MANAGER_H

// 2. 系统头文件
#include <Arduino.h>
#include <vector>

// 3. 第三方库头文件
#include <GxEPD2_BW.h>

// 4. 项目头文件
#include "Core/Event.h"
#include "Core/Card.h"

// 5. 类定义
class CardManager {
public:
    // 构造/析构
    CardManager();
    ~CardManager();

    // 公共方法
    void registerCard(Card* card);
    void switchCard(int index);

private:
    // 私有方法
    void _renderTransition();

    // 成员变量
    std::vector<Card*> _cards;
    int _currentIndex;
};

#endif // CARD_MANAGER_H
```

#### 注释规范

```cpp
/**
 * @brief 卡片管理器
 *
 * 负责管理所有卡片的注册、切换和渲染
 */
class CardManager {
public:
    /**
     * @brief 注册一张卡片
     *
     * @param card 卡片指针（不会获取所有权）
     */
    void registerCard(Card* card);

    /**
     * @brief 切换到指定卡片
     *
     * @param index 卡片索引（0-based）
     * @return true 切换成功
     * @return false 索引无效
     */
    bool switchCard(int index);
};
```

### 内存管理

```cpp
// ✅ 推荐：使用智能指针
std::unique_ptr<CardManager> cardManager;
cardManager = std::make_unique<CardManager>();

// ❌ 避免：裸指针（除非必要）
CardManager* cardManager = new CardManager();
delete cardManager;  // 容易忘记释放

// ✅ Lambda 按值捕获
String message = "Hello";
_epd.refreshFull([message](EPD_Class &d) {
    ChineseFont::drawString(d, 10, 10, message, GxEPD_BLACK);
});

// ❌ Lambda 捕获 this（可能悬空）
_epd.refreshFull([this](EPD_Class &d) {
    // 如果对象在刷新期间被销毁，会崩溃
});
```

### 错误处理

```cpp
// ✅ 检查返回值
if (!WiFi.begin(ssid, password)) {
    Serial.println("[ERROR] WiFi begin failed");
    return false;
}

// ✅ 检查指针
if (card == nullptr) {
    Serial.println("[ERROR] Card is null");
    return;
}

// ✅ 使用日志宏
LOG_DEBUG("[CardManager] Switching to card 0");
LOG_PRINTF("[CardManager] Card count: %d\n", _cards.size());
```

---

## 📤 提交规范

### Commit Message 格式

```
<type>(<scope>): <subject>

<body>

<footer>
```

#### Type 类型

| Type | 说明 | 示例 |
|------|------|------|
| `feat` | 新功能 | `feat(cards): 添加天气卡片` |
| `fix` | Bug 修复 | `fix(epd): 修复残影问题` |
| `refactor` | 重构 | `refactor(core): 重构事件系统` |
| `perf` | 性能优化 | `perf(render): 优化刷新速度` |
| `style` | 代码格式 | `style: 统一缩进为2空格` |
| `docs` | 文档更新 | `docs: 更新 README` |
| `test` | 测试相关 | `test: 添加事件系统测试` |
| `chore` | 构建/工具 | `chore: 更新依赖库版本` |

#### Scope 范围

- `cards` - 卡片系统
- `core` - 核心系统（事件、配置）
- `driver` - 硬件驱动
- `network` - 网络功能
- `ui` - UI 相关
- `tools` - 开发工具

#### 示例

```bash
# 好的提交信息
git commit -m "feat(cards): 添加 Lua 脚本卡片支持"
git commit -m "fix(epd): 修复局部刷新残影问题"
git commit -m "refactor(core): 使用智能指针管理内存"

# 不好的提交信息
git commit -m "update"
git commit -m "fix bug"
git commit -m "修改代码"
```

---

## 🔧 常用工具

### 字体转换工具

将 TTF 字体转换为 C 数组：

```bash
cd tools
python convert_font.py
```

配置文件：`tools/convert_font.py`

```python
# 字体文件路径
TTF_PATH = "../ttf/RenOuFangSong-16.ttf"

# 输出文件路径
OUTPUT_PATH = "../include/Fonts/HuiwenFangsong.h"

# 字体大小（像素）
FONT_SIZE = 16

# 字库文件列表
CHARSET_FILES = [
    "../ttf/常用字/level-1.txt",  # 一级常用字
]

# 额外的自定义字符
EXTRA_CHARS = [
    "设", "置", "系", "统", ...
]
```

### 图标生成工具

将 PNG 图标转换为二进制格式：

```bash
cd tools
python generate_icons.py
```

### 版本号管理

编辑 `include/Version.h`：

```cpp
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_BUILD 12  // 自动递增
```

版本号格式：`v1.0.0.12`

---

## 🐛 调试技巧

### 串口调试

```cpp
// 使用日志宏
LOG_DEBUG("[Module] Debug message");
LOG_PRINTF("[Module] Value: %d\n", value);

// 查看内存使用
Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
Serial.printf("PSRAM free: %d bytes\n", ESP.getFreePsram());

// 查看任务栈
Serial.printf("Stack high water mark: %d\n",
              uxTaskGetStackHighWaterMark(NULL));
```

### 常见问题排查

#### 编译错误

```bash
# 清理缓存重新编译
pio run -t clean
pio run

# 更新依赖库
pio lib update
```

#### 上传失败

```bash
# 检查串口
pio device list

# 手动指定串口
pio run -t upload --upload-port COM3

# 按住 BOOT 按钮再上传
```

#### 运行时崩溃

```bash
# 查看崩溃堆栈
pio device monitor

# 使用 ESP32 Exception Decoder
# 复制堆栈信息到解码器
```

---

## 📚 参考资源

### 官方文档

- [PlatformIO 文档](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/)
- [GxEPD2 库文档](https://github.com/ZinggJM/GxEPD2)

### 项目文档

- [架构设计](./ARCHITECTURE.md)
- [网络配置](./NETWORK_SETUP.md)
- [调试指南](./DEBUG_GUIDE.md)
- [OTA 更新](./OTA_UPDATE_GUIDE.md)

---

## 🤝 获取帮助

### 遇到问题？

1. **查阅文档** - 先查看 `docs/` 目录下的相关文档
2. **搜索 Issues** - 查看是否有类似问题
3. **提交 Issue** - 描述问题并附上日志
4. **讨论区** - 参与社区讨论

### 联系方式

- **GitHub Issues** - 报告 Bug 和功能请求
- **GitHub Discussions** - 技术讨论和问答

---

**文档版本**: v1.0
**最后更新**: 2026-02-08
**维护者**: Infinity Tag Team

---

💡 **提示**: 开始开发前，建议先阅读 [ARCHITECTURE.md](./ARCHITECTURE.md) 了解系统架构！
