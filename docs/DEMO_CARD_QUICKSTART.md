# DemoCard 快速开始指南

## 一、快速体验（5分钟）

### 1. 注册 DemoCard

在 `infinity-tag-esp32/src/main.cpp` 中添加：

```cpp
#include "Cards/DemoCard.h"

// 在 setup() 函数中
void setup() {
    // ... 现有初始化代码 ...

    // 创建并注册 DemoCard
    DemoCard* demoCard = new DemoCard();
    cardManager.registerCard(demoCard);

    // 如果想设置为默认卡片
    // cardManager.setCurrentCard(0);  // 根据实际索引调整
}
```

### 2. 编译并上传

```bash
cd infinity-tag-esp32
pio run --target upload
```

### 3. 操作说明

- **旋转编码器**：上下滚动文本（8行演示文本）
- **短按按钮**：切换背景图片开关（默认关闭）
- **长按按钮**：进入卡片切换模式

### 4. 预期效果

**无背景模式**：
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

## 二、添加背景图片（10分钟）

### 步骤 1：准备图片

找一张你喜欢的图片（任意格式、任意尺寸）

### 步骤 2：转换图片

```bash
# 安装 Python 依赖
pip install Pillow

# 转换图片
python tools/convert_image.py your_image.png demo_bg.bin

# 如果颜色反转，使用 --invert 选项
python tools/convert_image.py your_image.png demo_bg.bin --invert

# 调整二值化阈值（0-255，默认 128）
python tools/convert_image.py your_image.png demo_bg.bin --threshold 100
```

输出文件：
- `demo_bg.bin` - 用于上传到设备（2756 字节）
- `demo_bg_preview.png` - 预览效果

### 步骤 3：上传到设备

```bash
# 1. 将 demo_bg.bin 复制到 data/images/ 目录
mkdir -p infinity-tag-esp32/data/images
cp demo_bg.bin infinity-tag-esp32/data/images/

# 2. 上传文件系统
cd infinity-tag-esp32
pio run --target uploadfs

# 3. 重启设备
pio device monitor
```

### 步骤 4：测试

1. 进入 DemoCard
2. 短按按钮切换到"背景:开"
3. 应该看到背景图片 + 文字叠加效果

## 三、自定义你的卡片

### 修改文本内容

编辑 `src/Cards/DemoCard.cpp`：

```cpp
const char* DemoCard::DEMO_TEXTS[] = {
    "你的第一行文字",
    "你的第二行文字",
    "你的第三行文字",
    // ... 添加更多行
};

const int DemoCard::DEMO_TEXT_COUNT = 3;  // 更新行数
```

### 修改背景图片路径

```cpp
void DemoCard::renderToLayers(LayerManager& layerMgr) {
    // 修改这里的路径
    const char* bgPath = "/images/my_custom_bg.bin";
    // ...
}
```

### 修改布局

```cpp
// 修改行高
const int LINE_HEIGHT = 25;  // 默认 20

// 修改起始位置
const int START_Y = 20;  // 默认 10

// 修改文字对齐方式
// 居中对齐（默认）
int16_t x = (Framebuffer::WIDTH - textWidth) / 2;

// 左对齐
int16_t x = 10;

// 右对齐
int16_t x = Framebuffer::WIDTH - textWidth - 10;
```

## 四、进阶功能

### 1. 添加图标

```cpp
void DemoCard::renderToLayers(LayerManager& layerMgr) {
    // ... 背景层和文字层 ...

    // 添加图标层
    auto iconLayer = std::make_shared<Layer>(LayerType::CONTENT, 5);
    Framebuffer& fb = iconLayer->getFramebuffer();
    fb.clear(Framebuffer::WHITE);

    // 绘制一个简单的笑脸图标
    // 眼睛
    fb.fillRect(10, 10, 3, 3, Framebuffer::BLACK);
    fb.fillRect(20, 10, 3, 3, Framebuffer::BLACK);

    // 嘴巴（弧线）
    for (int x = 10; x <= 20; x++) {
        int y = 20 + (x - 15) * (x - 15) / 10;
        fb.setPixel(x, y, Framebuffer::BLACK);
    }

    layerMgr.addLayer(iconLayer);
}
```

### 2. 动态背景切换

```cpp
class DemoCard : public Card {
private:
    int _bgIndex;
    const char* _bgPaths[3] = {
        "/images/bg1.bin",
        "/images/bg2.bin",
        "/images/bg3.bin"
    };

public:
    void onEvent(const Event& event) override {
        if (event.type == EVENT_BUTTON_RELEASE) {
            _bgIndex = (_bgIndex + 1) % 3;
            _needsRefresh = true;
        }
    }

    void renderToLayers(LayerManager& layerMgr) override {
        auto bgLayer = std::make_shared<BackgroundLayer>(Framebuffer::WHITE);

        // 加载当前选中的背景
        if (_useBackground) {
            File file = LittleFS.open(_bgPaths[_bgIndex], "r");
            if (file) {
                uint8_t* bgData = (uint8_t*)malloc(Framebuffer::BUFFER_SIZE);
                if (bgData) {
                    file.read(bgData, Framebuffer::BUFFER_SIZE);
                    bgLayer->setImage(bgData, Framebuffer::WIDTH, Framebuffer::HEIGHT);
                    free(bgData);
                }
                file.close();
            }
        }

        layerMgr.addLayer(bgLayer);
    }
};
```

### 3. 添加动画效果

```cpp
class DemoCard : public Card {
private:
    int _animFrame;

public:
    void update() {
        // 在主循环中调用
        _animFrame = (millis() / 100) % 10;  // 每 100ms 一帧，共 10 帧
        _needsRefresh = true;
    }

    void renderToLayers(LayerManager& layerMgr) override {
        // 根据 _animFrame 绘制不同内容
        auto textLayer = std::make_shared<TextLayer>();

        int y = 50 + _animFrame * 2;  // 上下移动
        textLayer->addText(10, y, "动画文字", Framebuffer::BLACK);

        layerMgr.addLayer(textLayer);
    }
};
```

## 五、调试技巧

### 1. 查看串口日志

```bash
pio device monitor
```

查找以下日志：
- `[DemoCard] Entering demo card` - 卡片进入
- `[DemoCard] Background: ON/OFF` - 背景切换
- `[DemoCard] Line index: N` - 滚动位置
- `[DemoCard] Background image loaded` - 背景加载成功

### 2. 验证文件系统

```cpp
void setup() {
    // 列出所有图片文件
    File root = LittleFS.open("/images");
    File file = root.openNextFile();
    while (file) {
        Serial.printf("File: %s, Size: %d bytes\n",
                      file.name(), file.size());
        file = root.openNextFile();
    }
}
```

### 3. 测试背景图片

```cpp
void testBackground() {
    File file = LittleFS.open("/images/demo_bg.bin", "r");
    if (!file) {
        Serial.println("ERROR: File not found");
        return;
    }

    size_t size = file.size();
    Serial.printf("File size: %d bytes (expected: %d)\n",
                  size, Framebuffer::BUFFER_SIZE);

    if (size != Framebuffer::BUFFER_SIZE) {
        Serial.println("ERROR: Invalid file size");
        return;
    }

    Serial.println("Background file OK!");
    file.close();
}
```

## 六、常见问题

### Q1: 编译错误 "DemoCard.h: No such file"

**解决方案**：确保文件路径正确
```
infinity-tag-esp32/
├── include/
│   └── Cards/
│       └── DemoCard.h
└── src/
    └── Cards/
        └── DemoCard.cpp
```

### Q2: 背景图片不显示

**检查清单**：
1. 文件是否上传：`pio run --target uploadfs`
2. 文件大小是否正确：2756 字节
3. 路径是否正确：`/images/demo_bg.bin`
4. 是否切换到"背景:开"模式

### Q3: 文字显示不全

**原因**：行高或起始位置设置不当

**解决方案**：调整布局参数
```cpp
const int LINE_HEIGHT = 25;  // 增加行高
const int START_Y = 5;       // 调整起始位置
```

### Q4: 刷新闪烁

**原因**：刷新模式设置不当

**解决方案**：
```cpp
RefreshMode DemoCard::getRefreshMode() const {
    // 滚动时使用局部刷新（无闪烁）
    return RefreshMode::PARTIAL;

    // 切换背景时使用全屏刷新
    // return RefreshMode::FULL;
}
```

## 七、下一步

现在你已经掌握了：
- ✅ 创建自定义卡片
- ✅ 显示多行中文文字
- ✅ 导入和使用背景图片
- ✅ 使用图层系统

接下来可以尝试：
1. 创建更多自定义卡片（天气、日历、待办事项等）
2. 添加更多交互功能（按钮、编码器）
3. 实现动画效果
4. 集成网络功能（获取实时数据）

祝你玩得开心！🎉
