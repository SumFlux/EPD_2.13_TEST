# 从图片生成卡片图标工具

## 功能特性

✅ **支持多种图片格式**：PNG、JPG、BMP、GIF、**SVG** 等
✅ **支持SVG代码**：直接从SVG代码字符串生成图标
✅ **自动等比缩放**：保持图片宽高比，居中显示
✅ **智能处理透明度**：自动将透明背景转换为白色
✅ **可调节黑白阈值**：控制图标的黑白效果
✅ **生成预览图**：放大8倍，方便查看最终效果
✅ **生成bin文件**：直接用于ESP32设备

## 安装依赖

### 基本依赖（必需）

```bash
pip install pillow
```

### SVG支持（可选）

如果需要使用SVG功能：

```bash
pip install cairosvg
```

**Windows用户注意**：可能还需要安装 GTK+ 运行时
- 下载地址：https://github.com/tschoonj/GTK-for-Windows-Runtime-Environment-Installer

## 快速开始

### 1. 从PNG/JPG生成

```bash
cd infinity-tag-esp32/tools
python create_icon_from_image.py icon.png settings
```

### 2. 从SVG文件生成

```bash
python create_icon_from_image.py icon.svg settings
```

### 3. 从SVG代码生成

```bash
# 方法1：直接传入SVG代码
python create_icon_from_image.py --svg-code '<svg width="100" height="100">...</svg>' settings

# 方法2：从文件读取SVG代码
python create_icon_from_image.py --svg-code "$(cat icon.svg)" settings
```

## SVG使用示例

### 示例1：简单的齿轮图标

```bash
python create_icon_from_image.py --svg-code '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M12 15.5A3.5 3.5 0 0 1 8.5 12 3.5 3.5 0 0 1 12 8.5a3.5 3.5 0 0 1 3.5 3.5 3.5 3.5 0 0 1-3.5 3.5m7.43-2.53c.04-.32.07-.64.07-.97 0-.33-.03-.66-.07-1l2.11-1.63c.19-.15.24-.42.12-.64l-2-3.46c-.12-.22-.39-.31-.61-.22l-2.49 1c-.52-.39-1.06-.73-1.69-.98l-.37-2.65A.506.506 0 0 0 14 2h-4c-.25 0-.46.18-.5.42l-.37 2.65c-.63.25-1.17.59-1.69.98l-2.49-1c-.22-.09-.49 0-.61.22l-2 3.46c-.13.22-.07.49.12.64L4.57 11c-.04.34-.07.67-.07 1 0 .33.03.65.07.97l-2.11 1.66c-.19.15-.25.42-.12.64l2 3.46c.12.22.39.3.61.22l2.49-1.01c.52.4 1.06.74 1.69.99l.37 2.65c.04.24.25.42.5.42h4c.25 0 .46-.18.5-.42l.37-2.65c.63-.26 1.17-.59 1.69-.99l2.49 1.01c.22.08.49 0 .61-.22l2-3.46c.12-.22.07-.49-.12-.64l-2.11-1.66z"/></svg>' settings
```

### 示例2：WiFi图标

```bash
python create_icon_from_image.py --svg-code '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M12 21l-8-8c4.4-4.4 11.6-4.4 16 0l-8 8zm0-4l-4-4c2.2-2.2 5.8-2.2 8 0l-4 4zm0-4l-2-2c1.1-1.1 2.9-1.1 4 0l-2 2z"/></svg>' wifi
```

### 示例3：从SVG文件生成（推荐）

创建一个 `settings.svg` 文件：

```xml
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
  <path d="M19.14,12.94c0.04-0.3,0.06-0.61,0.06-0.94c0-0.32-0.02-0.64-0.07-0.94l2.03-1.58c0.18-0.14,0.23-0.41,0.12-0.61 l-1.92-3.32c-0.12-0.22-0.37-0.29-0.59-0.22l-2.39,0.96c-0.5-0.38-1.03-0.7-1.62-0.94L14.4,2.81c-0.04-0.24-0.24-0.41-0.48-0.41 h-3.84c-0.24,0-0.43,0.17-0.47,0.41L9.25,5.35C8.66,5.59,8.12,5.92,7.63,6.29L5.24,5.33c-0.22-0.08-0.47,0-0.59,0.22L2.74,8.87 C2.62,9.08,2.66,9.34,2.86,9.48l2.03,1.58C4.84,11.36,4.8,11.69,4.8,12s0.02,0.64,0.07,0.94l-2.03,1.58 c-0.18,0.14-0.23,0.41-0.12,0.61l1.92,3.32c0.12,0.22,0.37,0.29,0.59,0.22l2.39-0.96c0.5,0.38,1.03,0.7,1.62,0.94l0.36,2.54 c0.05,0.24,0.24,0.41,0.48,0.41h3.84c0.24,0,0.44-0.17,0.47-0.41l0.36-2.54c0.59-0.24,1.13-0.56,1.62-0.94l2.39,0.96 c0.22,0.08,0.47,0,0.59-0.22l1.92-3.32c0.12-0.22,0.07-0.47-0.12-0.61L19.14,12.94z M12,15.6c-1.98,0-3.6-1.62-3.6-3.6 s1.62-3.6,3.6-3.6s3.6,1.62,3.6,3.6S13.98,15.6,12,15.6z"/>
</svg>
```

然后运行：

```bash
python create_icon_from_image.py settings.svg settings
```

## 高级选项

#### 1. 调整黑白阈值

阈值范围：0-255（默认：128）
- **阈值越小** → 更多黑色（适合浅色图片）
- **阈值越大** → 更多白色（适合深色图片）

```bash
# 更多黑色（适合浅色图标）
python create_icon_from_image.py icon.png settings --threshold 100

# 更多白色（适合深色图标）
python create_icon_from_image.py icon.png settings --threshold 180
```

#### 2. 不添加边框

```bash
python create_icon_from_image.py icon.png settings --no-border
```

#### 3. 自定义尺寸

```bash
python create_icon_from_image.py icon.png settings --size 64
```

## 完整工作流程

### 步骤1：准备图片

推荐图片特征：
- ✅ 高对比度（黑白分明）
- ✅ 简单图标（避免复杂细节）
- ✅ 正方形或接近正方形
- ✅ 分辨率：至少 48x48，推荐 256x256 或更高

### 步骤2：生成图标

```bash
cd infinity-tag-esp32/tools
python create_icon_from_image.py my_icon.png settings
```

### 步骤3：查看预览

打开生成的预览图：
```
data/icons/preview/settings_preview.png
```

如果效果不满意，调整阈值重新生成：
```bash
# 太多白色？降低阈值
python create_icon_from_image.py my_icon.png settings --threshold 100

# 太多黑色？提高阈值
python create_icon_from_image.py my_icon.png settings --threshold 180
```

### 步骤4：上传到设备

```bash
cd ..
pio run -t uploadfs
```

## 输出文件说明

### 1. bin文件（ESP32使用）

- **路径**：`data/icons/card_<名称>.bin`
- **大小**：288字节（48x48像素，1-bit）
- **格式**：原始二进制位图
- **用途**：上传到ESP32的LittleFS文件系统

### 2. 预览图（查看效果）

- **路径**：`data/icons/preview/<名称>_preview.png`
- **大小**：384x384像素（放大8倍）
- **格式**：PNG图片
- **用途**：预览最终在墨水屏上的显示效果

## 使用示例

### 示例1：生成设置图标

```bash
# 假设你有一个 settings_icon.png
python create_icon_from_image.py settings_icon.png settings

# 输出：
# ✅ data/icons/card_settings.bin
# ✅ data/icons/preview/settings_preview.png
```

### 示例2：生成WiFi图标（调整阈值）

```bash
# 第一次尝试（默认阈值）
python create_icon_from_image.py wifi_icon.png wifi

# 查看预览，发现太多白色，降低阈值
python create_icon_from_image.py wifi_icon.png wifi --threshold 100

# 查看预览，效果满意！
```

### 示例3：批量生成多个图标

创建一个批处理脚本 `generate_all_icons.sh`：

```bash
#!/bin/bash
cd infinity-tag-esp32/tools

# 生成所有图标
python create_icon_from_image.py icons/settings.png settings --threshold 120
python create_icon_from_image.py icons/wifi.png wifi --threshold 100
python create_icon_from_image.py icons/calendar.png calendar --threshold 130
python create_icon_from_image.py icons/image.png image --threshold 110

# 上传到设备
cd ..
pio run -t uploadfs

echo "✅ 所有图标已生成并上传！"
```

## 常见问题

### Q1: 图标显示全黑或全白？

**原因**：阈值设置不合适

**解决**：
```bash
# 全黑 → 提高阈值
python create_icon_from_image.py icon.png name --threshold 180

# 全白 → 降低阈值
python create_icon_from_image.py icon.png name --threshold 80
```

### Q2: 图标细节丢失？

**原因**：原图太复杂或分辨率太低

**解决**：
1. 使用更简单的图标
2. 使用更高分辨率的原图
3. 手动编辑图片，增强对比度

### Q3: 图标变形？

**原因**：原图不是正方形

**解决**：脚本会自动等比缩放并居中，不会变形。如果觉得留白太多，可以先裁剪原图为正方形。

### Q4: 透明背景变成黑色？

**原因**：脚本默认将透明背景转换为白色

**解决**：这是正常行为。如果需要黑色背景，可以先在图片编辑器中将透明背景改为黑色。

## 技术细节

### 图标格式

- **尺寸**：48x48 像素
- **颜色深度**：1-bit（黑白）
- **存储格式**：每8个像素打包为1个字节
- **字节顺序**：从左到右，从上到下
- **位顺序**：LSB first（最低位在前）

### 像素编码

```
0 = 黑色像素
1 = 白色像素
```

### 文件大小计算

```
48 × 48 = 2304 像素
2304 ÷ 8 = 288 字节
```

## 图标设计建议

### ✅ 推荐

- 简单的几何图形
- 高对比度
- 粗线条（至少2-3像素宽）
- 留有适当留白
- 使用矢量图标（SVG）导出为高分辨率PNG

### ❌ 避免

- 复杂的渐变
- 细小的文字
- 过于细腻的细节
- 低对比度的颜色
- 过于密集的图案

## 相关文件

- `generate_icons.py` - 从文字生成图标
- `create_icon_from_image.py` - 从图片生成图标（本工具）
- `data/icons/` - 图标bin文件目录
- `data/icons/preview/` - 预览图目录

## 更新日志

### v1.0.0 (2026-02-09)
- ✅ 初始版本
- ✅ 支持从图片生成图标
- ✅ 自动等比缩放
- ✅ 可调节黑白阈值
- ✅ 生成预览图
- ✅ 处理透明背景
