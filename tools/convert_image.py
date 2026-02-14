#!/usr/bin/env python3
"""
将图片转换为墨水屏背景格式
输出：104×212 像素，1位单色，2756 字节
"""

from PIL import Image
import sys
import os

def convert_image_to_epd(input_path, output_path, threshold=128, invert=False):
    """
    转换图片为墨水屏格式

    Args:
        input_path: 输入图片路径（支持 PNG, JPG 等）
        output_path: 输出二进制文件路径（.bin）
        threshold: 二值化阈值（0-255，默认 128）
        invert: 是否反转黑白（默认 False）
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
    # 阈值：大于阈值为白色（1），小于等于阈值为黑色（0）
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

            if invert:
                # 反转模式
                if pixel == 0:  # 黑色 → 白色
                    buffer[byte_index] |= (1 << bit_index)
                else:  # 白色 → 黑色
                    buffer[byte_index] &= ~(1 << bit_index)
            else:
                # 正常模式
                if pixel == 0:  # 黑色
                    buffer[byte_index] &= ~(1 << bit_index)
                else:  # 白色
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

    # 8. 显示统计信息
    black_pixels = sum(1 for y in range(height) for x in range(width) if pixels[x, y] == 0)
    white_pixels = width * height - black_pixels
    print(f"  黑色像素: {black_pixels} ({black_pixels * 100 / (width * height):.1f}%)")
    print(f"  白色像素: {white_pixels} ({white_pixels * 100 / (width * height):.1f}%)")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("用法: python convert_image.py <输入图片> [输出文件] [选项]")
        print("")
        print("参数:")
        print("  输入图片    - 要转换的图片文件（PNG, JPG 等）")
        print("  输出文件    - 输出的 .bin 文件（可选，默认自动生成）")
        print("")
        print("选项:")
        print("  --threshold N  - 二值化阈值（0-255，默认 128）")
        print("  --invert       - 反转黑白颜色")
        print("")
        print("示例:")
        print("  python convert_image.py background.png")
        print("  python convert_image.py background.png demo_bg.bin")
        print("  python convert_image.py background.png --threshold 100")
        print("  python convert_image.py background.png --invert")
        sys.exit(1)

    input_path = sys.argv[1]

    # 解析参数
    output_path = None
    threshold = 128
    invert = False

    i = 2
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg == '--threshold' and i + 1 < len(sys.argv):
            threshold = int(sys.argv[i + 1])
            i += 2
        elif arg == '--invert':
            invert = True
            i += 1
        elif output_path is None and not arg.startswith('--'):
            output_path = arg
            i += 1
        else:
            i += 1

    # 自动生成输出文件名
    if output_path is None:
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        output_path = f"{base_name}_epd.bin"

    if not os.path.exists(input_path):
        print(f"错误: 文件不存在: {input_path}")
        sys.exit(1)

    print(f"转换参数:")
    print(f"  输入文件: {input_path}")
    print(f"  输出文件: {output_path}")
    print(f"  二值化阈值: {threshold}")
    print(f"  反转颜色: {'是' if invert else '否'}")
    print("")

    convert_image_to_epd(input_path, output_path, threshold, invert)
