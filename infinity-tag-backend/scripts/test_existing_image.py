"""
测试已有图片的位图转换
验证修复后的接口是否正确返回 2808 字节（行对齐格式）
"""
import sys
import os

# 添加项目根目录到 Python 路径
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from app.services.image_processing import ImageProcessor
from pathlib import Path

def test_existing_images():
    """测试 custom_images 目录下的所有图片"""
    assets_dir = Path(__file__).parent.parent / "assets" / "custom_images"

    if not assets_dir.exists():
        print("未找到 custom_images 目录")
        return

    print("=" * 60)
    print("测试已有图片的位图转换")
    print("=" * 60)

    image_count = 0
    success_count = 0

    # 遍历所有用户目录
    for user_dir in assets_dir.iterdir():
        if not user_dir.is_dir():
            continue

        print(f"\n[用户 {user_dir.name}]")

        # 遍历该用户的所有图片
        for image_file in user_dir.glob("*.png"):
            image_count += 1
            print(f"  测试: {image_file.name}")

            try:
                # 使用修复后的方法转换
                bitmap = ImageProcessor.get_epd_bitmap(str(image_file))

                # 验证大小
                expected_size = 2808  # 行对齐: 27 bytes/row * 104 rows
                actual_size = len(bitmap)

                if actual_size == expected_size:
                    print(f"    ✓ 大小正确: {actual_size} 字节")
                    success_count += 1
                else:
                    print(f"    ✗ 大小错误: {actual_size} 字节 (期望 {expected_size})")

            except Exception as e:
                print(f"    ✗ 转换失败: {e}")

    print("\n" + "=" * 60)
    print(f"测试完成: {success_count}/{image_count} 张图片转换成功")
    print("=" * 60)

    if image_count == 0:
        print("\n提示: 未找到已上传的图片，可以先上传一些图片进行测试")

if __name__ == "__main__":
    test_existing_images()
