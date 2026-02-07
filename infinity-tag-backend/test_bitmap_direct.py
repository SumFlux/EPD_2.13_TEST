"""
直接测试位图转换函数
"""
import sys
from pathlib import Path

# 添加项目路径
sys.path.insert(0, str(Path(__file__).parent))

from app.services.image_processing import ImageProcessor
from PIL import Image

# 创建一个测试图片
print("创建测试图片 212x104...")
img = Image.new('1', (212, 104), color=1)
test_path = "test_212x104_temp.png"
img.save(test_path)

# 测试转换
print("测试位图转换...")
try:
    bitmap = ImageProcessor.get_epd_bitmap(test_path)
    print("SUCCESS: Conversion completed!")
    print(f"  Output size: {len(bitmap)} bytes")
    print(f"  Expected size: 2808 bytes")

    if len(bitmap) == 2808:
        print("PASS: Size is correct!")
    else:
        print(f"FAIL: Size mismatch! Difference: {len(bitmap) - 2808} bytes")

except Exception as e:
    print(f"ERROR: Conversion failed: {e}")

# 清理
import os
os.remove(test_path)
