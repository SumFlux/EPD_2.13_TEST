"""
测试位图转换逻辑
验证 212x104 图片转换为 2808 字节的 1-bit 位图（行对齐格式）
"""
from PIL import Image, ImageDraw
import io

def create_test_image():
    """创建一个 212x104 的测试图片"""
    img = Image.new('1', (212, 104), color=1)  # 白色背景
    draw = ImageDraw.Draw(img)

    # 画一些黑色图案用于测试
    draw.rectangle([10, 10, 50, 50], fill=0)  # 黑色方块
    draw.ellipse([60, 10, 100, 50], fill=0)   # 黑色圆
    draw.text((110, 20), "TEST", fill=0)      # 黑色文字

    return img

def get_epd_bitmap_current(img: Image.Image) -> bytes:
    """当前的实现"""
    img = img.convert('1')
    width, height = img.size
    pixels = img.load()
    buffer = bytearray()

    for y in range(height):
        byte_val = 0
        for x in range(width):
            pixel = pixels[x, y]
            # 0=Black, 255=White. EPD: 0=Black, 1=White
            bit = 1 if pixel > 127 else 0

            # Packing: MSB first
            bit_pos = 7 - (x % 8)
            if bit:
                byte_val |= (1 << bit_pos)

            if (x % 8) == 7:
                buffer.append(byte_val)
                byte_val = 0

        # Handle padding if width is not multiple of 8
        if (width % 8) != 0:
            buffer.append(byte_val)

    return bytes(buffer)

def get_epd_bitmap_fixed(img: Image.Image) -> bytes:
    """修复后的实现 - 行对齐格式，总共 2808 字节"""
    img = img.convert('1')
    width, height = img.size
    pixels = img.load()
    buffer = bytearray()

    # 紧密打包：逐位处理，不按行对齐
    bit_index = 0
    byte_val = 0

    for y in range(height):
        for x in range(width):
            pixel = pixels[x, y]
            # PIL '1' mode: 0=black, 255=white
            # EPD: 0=black, 1=white
            bit = 1 if pixel > 127 else 0

            # MSB first packing
            bit_pos = 7 - (bit_index % 8)
            if bit:
                byte_val |= (1 << bit_pos)

            bit_index += 1

            # 每 8 位输出一个字节
            if bit_index % 8 == 0:
                buffer.append(byte_val)
                byte_val = 0

    # 处理最后不足 8 位的情况
    if bit_index % 8 != 0:
        buffer.append(byte_val)

    return bytes(buffer)

def main():
    print("=" * 60)
    print("位图转换测试")
    print("=" * 60)

    # 创建测试图片
    img = create_test_image()
    print(f"图片尺寸: {img.size[0]}x{img.size[1]}")

    # 保存测试图片
    img.save("test_212x104.png")
    print("已保存测试图片: test_212x104.png")

    # 测试当前实现
    print("\n[当前实现]")
    bitmap_current = get_epd_bitmap_current(img)
    print(f"输出大小: {len(bitmap_current)} 字节")
    print(f"前16字节: {bitmap_current[:16].hex()}")

    # 测试修复后的实现
    print("\n[修复后实现]")
    bitmap_fixed = get_epd_bitmap_fixed(img)
    print(f"输出大小: {len(bitmap_fixed)} 字节")
    print(f"前16字节: {bitmap_fixed[:16].hex()}")

    # 验证
    print("\n[验证]")
    expected_size = (212 + 7) // 8 * 104  # 2808 字节 (行对齐)
    print(f"预期大小: {expected_size} 字节")

    if len(bitmap_current) == expected_size:
        print("当前实现大小正确")
    else:
        print(f"当前实现大小错误 (差异: {len(bitmap_current) - expected_size} 字节)")

    if len(bitmap_fixed) == expected_size:
        print("修复后实现大小正确")
    else:
        print(f"修复后实现大小错误 (差异: {len(bitmap_fixed) - expected_size} 字节)")

    # 比较两种实现
    if bitmap_current == bitmap_fixed:
        print("\n两种实现输出相同")
    else:
        diff_count = sum(a != b for a, b in zip(bitmap_current, bitmap_fixed))
        print(f"\n两种实现输出不同 (差异字节数: {diff_count})")

if __name__ == "__main__":
    main()
