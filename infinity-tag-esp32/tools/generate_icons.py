#!/usr/bin/env python3
"""
卡片图标生成工具

生成48x48的1-bit位图图标，用于卡片切换界面
"""

from PIL import Image, ImageDraw, ImageFont
import os

def create_icon(text, output_path, size=48):
    """
    创建一个简单的文字图标

    Args:
        text: 图标上的文字（1-2个字符）
        output_path: 输出文件路径
        size: 图标尺寸（默认48x48）
    """
    # 创建白色背景图像
    img = Image.new('1', (size, size), 1)  # 1-bit模式，1=白色
    draw = ImageDraw.Draw(img)

    # 绘制边框
    draw.rectangle([0, 0, size-1, size-1], outline=0, width=2)

    # 绘制文字（居中）
    try:
        # 尝试使用系统字体
        font = ImageFont.truetype("arial.ttf", size//2)
    except:
        # 如果没有找到字体，使用默认字体
        font = ImageFont.load_default()

    # 计算文字位置（居中）
    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    x = (size - text_width) // 2
    y = (size - text_height) // 2

    # 绘制文字
    draw.text((x, y), text, fill=0, font=font)

    # 保存为二进制文件
    pixels = list(img.getdata())

    # 转换为字节数组（每8个像素一个字节）
    bytes_data = []
    for i in range(0, len(pixels), 8):
        byte = 0
        for j in range(8):
            if i + j < len(pixels):
                # 0=黑色，1=白色
                if pixels[i + j] == 0:
                    byte |= (1 << j)
        bytes_data.append(byte)

    # 写入文件
    with open(output_path, 'wb') as f:
        f.write(bytes(bytes_data))

    print(f"Created icon: {output_path} ({len(bytes_data)} bytes)")

def create_default_icons():
    """创建默认图标"""

    # 创建输出目录
    os.makedirs('data/icons', exist_ok=True)

    # 创建各种卡片图标
    icons = [
        ('WiFi', 'data/icons/card_wifi.bin'),
        ('设置', 'data/icons/card_settings.bin'),
        ('图片', 'data/icons/card_image.bin'),
        ('黄历', 'data/icons/card_calendar.bin'),
        ('木鱼', 'data/icons/card_muyu.bin'),
        ('默认', 'data/icons/card_default.bin'),
    ]

    for text, path in icons:
        create_icon(text, path)

    print(f"\n✅ Created {len(icons)} default icons")
    print("\n上传到设备：")
    print("pio run -t uploadfs")

if __name__ == '__main__':
    create_default_icons()
