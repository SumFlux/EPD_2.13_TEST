# 墨水屏背景图片导入教程

## 一、背景图片格式要求

### 1. 尺寸要求
- **宽度**：104 像素
- **高度**：212 像素
- **格式**：1位单色（黑白）
- **文件大小**：2756 字节（104×212÷8）

### 2. 颜色要求
- 只支持黑白两色
- 黑色：0（像素值为 0）
- 白色：1（像素值为 1）

## 二、准备背景图片

### 方法 1：使用 Python 脚本转换（推荐）

创建 `tools/convert_image.py`：

```python
#!/usr/bin/env python3
"""
将图片转换为墨水屏背景格式
输出：104×212 像素，1位单色，2756 字节
"""

from PIL import Image
import sys
import os

def convert_image_to_epd(input_path, output_path):
    """
    转换图片为墨水屏格式

    Args:
        input_path: 输入图片路径（支持 PNG, JPG 等）
        output_path: 输出二进制文件路径（.bin）
    """
    # 1. 打开图片
    img = Image.open(input_path)
    print(f"原始图片尺寸: {img.size}")

    # 2. 调整尺寸为 104×212
    img = img.resize((104, 212), Image.Resampling.LANCZOS)
    print(f"调整后尺寸: {img.size}")

    # 3. 转换为灰度图
    img = img.convert('L')

    # 4. 二值化（黑白）
    # 阈值 128：大于 128 为白色（1），小于等于 128 为黑色（0）
    threshold = 128
    img = img.point(lambda x: 255 if x > threshold else 0, mode='1')

    # 5. 转换为字节数组
    # 格式：每行从左到右，每 8 个像素打包成 1 字节
    # MSB first：最左边的像素是最高位
    width, height = img.size
    bytes_per_row = (width + 7) // 8  # 104 / 8 = 13 字节/行

    buffer = bytearray(bytes_per_row * height)

    pixels = img.load()
    for y in range(height):
        for x in range(width):
            byte_index = y * bytes_per_row + (x // 8)
            bit_index = 7 - (x % 8)  # MSB first

            # 获取像素值（0=黑色，255=白色）
            pixel = pixels[x, y]

            if pixel == 0:  # 黑色
                # 位设为 0（黑色）
                buffer[byte_index] &= ~(1 << bit_index)
            else:  # 白色
                # 位设为 1（白色）
                buffer[byte_index] |= (1 << bit_index)

    # 6. 保存为二进制文件
    with open(output_path, 'wb') as f:
        f.write(buffer)

    print(f"✓ 转换完成！")
    print(f"  输出文件: {output_path}")
    print(f"  文件大小: {len(buffer)} 字节")

    # 7. 生成预览（可选）
    preview_path = output_path.replace('.bin', '_preview.png')
    img.save(preview_path)
    print(f"  预览图片: {preview_path}")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("用法: python convert_image.py <输入图片> [输出文件]")
        print("示例: python convert_image.py background.png demo_bg.bin")
        sys.exit(1)

    input_path = sys.argv[1]

    if len(sys.argv) >= 3:
        output_path = sys.argv[2]
    else:
        # 自动生成输出文件名
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        output_path = f"{base_name}_epd.bin"

    if not os.path.exists(input_path):
        print(f"错误: 文件不存在: {input_path}")
        sys.exit(1)

    convert_image_to_epd(input_path, output_path)
```

### 使用方法：

```bash
# 安装依赖
pip install Pillow

# 转换图片
python tools/convert_image.py background.png demo_bg.bin

# 输出：
# - demo_bg.bin (2756 字节，用于上传到设备)
# - demo_bg_preview.png (预览图片)
```

### 方法 2：使用 GIMP 手动转换

1. 打开 GIMP
2. 导入图片
3. 图像 → 缩放图像 → 设置为 104×212 像素
4. 图像 → 模式 → 灰度
5. 图像 → 模式 → 索引 → 使用黑白（1位）调色板
6. 文件 → 导出为 → 选择 "原始图像数据" (.data)
7. 手动重命名为 .bin

## 三、上传背景图片到设备

### 方法 1：使用 PlatformIO 文件系统上传

1. 将 `.bin` 文件放到 `infinity-tag-esp32/data/images/` 目录：

```
infinity-tag-esp32/
├── data/
│   ├── images/
│   │   ├── demo_bg.bin       # 你的背景图片
│   │   └── other_bg.bin      # 其他背景图片
│   ├── cards/
│   └── icons/
```

2. 上传文件系统到设备：

```bash
cd infinity-tag-esp32
pio run --target uploadfs
```

3. 验证上传成功：

在 `main.cpp` 中添加测试代码：

```cpp
void setup() {
    Serial.begin(115200);

    // 初始化 LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
        return;
    }

    // 列出所有文件
    File root = LittleFS.open("/images");
    File file = root.openNextFile();
    while (file) {
        Serial.printf("File: %s, Size: %d bytes\n",
                      file.name(), file.size());
        file = root.openNextFile();
    }
}
```

### 方法 2：通过 Web 界面上传（如果已实现）

如果你的设备有 Web 文件管理界面：

1. 连接设备 WiFi
2. 访问设备 IP 地址
3. 进入文件管理页面
4. 上传 `.bin` 文件到 `/images/` 目录

## 四、在卡片中使用背景图片

### 示例 1：DemoCard（已创建）

```cpp
void DemoCard::renderToLayers(LayerManager& layerMgr) {
    // 1. 创建背景层
    auto bgLayer = std::make_shared<BackgroundLayer>(Framebuffer::WHITE);

    // 2. 加载背景图片
    if (_useBackground) {
        const char* bgPath = "/images/demo_bg.bin";

        if (LittleFS.exists(bgPath)) {
            File file = LittleFS.open(bgPath, "r");
            if (file && file.size() == Framebuffer::BUFFER_SIZE) {
                uint8_t* bgData = (uint8_t*)malloc(Framebuffer::BUFFER_SIZE);
                if (bgData) {
                    file.read(bgData, Framebuffer::BUFFER_SIZE);
                    bgLayer->setImage(bgData,
                                     Framebuffer::WIDTH,
                                     Framebuffer::HEIGHT);
                    free(bgData);
                }
                file.close();
            }
        }
    }

    layerMgr.addLayer(bgLayer);

    // 3. 添加其他图层（文字、图标等）
    // ...
}
```

### 示例 2：简化版（直接绘制到 Framebuffer）

```cpp
void MyCard::renderToLayers(LayerManager& layerMgr) {
    auto bgLayer = std::make_shared<BackgroundLayer>(Framebuffer::WHITE);

    // 直接从文件加载到 framebuffer
    File file = LittleFS.open("/images/my_bg.bin", "r");
    if (file) {
        Framebuffer& fb = bgLayer->getFramebuffer();
        file.read(fb.getBuffer(), Framebuffer::BUFFER_SIZE);
        file.close();
    }

    layerMgr.addLayer(bgLayer);
}
```

## 五、背景图片设计建议

### 1. 对比度
- 墨水屏只有黑白两色，确保图片对比度足够
- 避免过多细节，简洁的图案效果更好

### 2. 内容区域
- 预留文字显示区域（建议中间 80×180 区域留白）
- 装饰性元素放在边缘

### 3. 刷新策略
- 背景图片变化时使用 `RefreshMode::FULL`（全屏刷新）
- 只有文字变化时使用 `RefreshMode::PARTIAL`（局部刷新）

### 4. 示例布局

```
┌────────────────────────┐  104px
│  ╔════════════════╗    │
│  ║                ║    │  ← 装饰边框
│  ║                ║    │
│  ║   文字区域      ║    │  ← 留白区域
│  ║                ║    │
│  ║                ║    │
│  ╚════════════════╝    │
└────────────────────────┘
        212px
```

## 六、测试 DemoCard

### 1. 注册卡片

在 `main.cpp` 中注册 DemoCard：

```cpp
#include "Cards/DemoCard.h"

void setup() {
    // ... 其他初始化代码 ...

    // 创建并注册 DemoCard
    DemoCard* demoCard = new DemoCard();
    cardManager.registerCard(demoCard);

    // 设置为当前卡片
    cardManager.setCurrentCard(0);  // 假设是第一张卡片
}
```

### 2. 操作说明

- **短按按钮**：切换背景图片开关
- **旋转编码器**：上下滚动文本
- **长按按钮**：进入卡片切换模式

### 3. 预期效果

- 无背景：白色背景 + 黑色文字
- 有背景：背景图片 + 黑色文字（图层叠加）
- 滚动时：显示上下箭头指示器

## 七、常见问题

### Q1: 背景图片不显示？

**检查清单**：
1. 文件是否存在：`LittleFS.exists("/images/demo_bg.bin")`
2. 文件大小是否正确：应该是 2756 字节
3. 文件系统是否上传：`pio run --target uploadfs`
4. 串口日志是否有错误信息

### Q2: 背景图片颜色反转？

**原因**：位格式定义不一致

**解决方案**：修改转换脚本中的位逻辑：

```python
# 如果颜色反转，交换黑白逻辑
if pixel == 0:  # 黑色
    buffer[byte_index] |= (1 << bit_index)  # 改为设置位
else:  # 白色
    buffer[byte_index] &= ~(1 << bit_index)  # 改为清除位
```

### Q3: 背景图片模糊或失真？

**原因**：缩放算法不佳

**解决方案**：
1. 使用高质量缩放算法（LANCZOS）
2. 原始图片尽量接近 104×212 比例
3. 手动调整图片细节后再转换

### Q4: 内存不足？

**原因**：临时分配 2756 字节可能失败

**解决方案**：
```cpp
// 使用 PSRAM 分配
uint8_t* bgData = (uint8_t*)heap_caps_malloc(
    Framebuffer::BUFFER_SIZE,
    MALLOC_CAP_SPIRAM
);
```

## 八、进阶技巧

### 1. 动态背景切换

```cpp
class MyCard : public Card {
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
            // 触发重新渲染
        }
    }
};
```

### 2. 半透明效果（抖动）

```cpp
// 在 BackgroundLayer 中实现半透明效果
void drawSemiTransparent(Framebuffer& fb, uint8_t alpha) {
    // 使用抖动算法模拟半透明
    for (int y = 0; y < Framebuffer::HEIGHT; y++) {
        for (int x = 0; x < Framebuffer::WIDTH; x++) {
            if ((x + y) % 2 == 0) {  // 棋盘抖动
                fb.setPixel(x, y, Framebuffer::BLACK);
            }
        }
    }
}
```

### 3. 背景缓存

```cpp
class MyCard : public Card {
private:
    static uint8_t* _cachedBg;  // 静态缓存

    void loadBackgroundOnce() {
        if (_cachedBg == nullptr) {
            _cachedBg = (uint8_t*)heap_caps_malloc(
                Framebuffer::BUFFER_SIZE,
                MALLOC_CAP_SPIRAM
            );
            // 只加载一次
            File file = LittleFS.open("/images/bg.bin", "r");
            file.read(_cachedBg, Framebuffer::BUFFER_SIZE);
            file.close();
        }
    }
};
```

## 九、资源推荐

### 免费图标/图案网站
- [Flaticon](https://www.flaticon.com/) - 免费图标
- [Pixabay](https://pixabay.com/) - 免费图片
- [Unsplash](https://unsplash.com/) - 高质量照片

### 图片处理工具
- **GIMP** - 免费开源图像编辑器
- **ImageMagick** - 命令行图像处理工具
- **Pillow (Python)** - Python 图像处理库

### 在线工具
- [Online Image Converter](https://www.online-convert.com/) - 在线格式转换
- [Photopea](https://www.photopea.com/) - 在线 Photoshop 替代品

---

现在你已经掌握了如何在墨水屏上显示中文和导入背景图片！🎉
