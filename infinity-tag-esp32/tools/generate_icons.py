#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

    # 尝试加载中文字体
    font = None

    # 获取脚本所在目录
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    font_paths = [
        # 项目指定的中文字体（使用绝对路径）
        os.path.join(project_root, "ttf", "ChangBanDianSong-12.ttf"),
        os.path.join(project_root, "ttf", "RenOuFangSong-16.ttf"),
        os.path.join(project_root, "ttf", "匯文仿宋.ttf"),
        # Windows 中文字体（备用）
        "C:/Windows/Fonts/msyh.ttc",      # 微软雅黑
        "C:/Windows/Fonts/simhei.ttf",    # 黑体
        "C:/Windows/Fonts/simsun.ttc",    # 宋体
        # Linux 中文字体
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
        "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
    ]

    for font_path in font_paths:
        try:
            font = ImageFont.truetype(font_path, size//3)
            print(f"Using font: {font_path}")
            break
        except Exception as e:
            continue

    if font is None:
        print("Warning: No Chinese font found, using default font")
        font = ImageFont.load_default()

    # 计算文字位置（居中）
    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    x = (size - text_width) // 2 - bbox[0]
    y = (size - text_height) // 2 - bbox[1]

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

    print(f"\nCreated {len(icons)} default icons")
    print("\n上传到设备：")
    print("pio run -t uploadfs")

if __name__ == '__main__':
    create_default_icons()
