"""
测试 bitmap API 接口的实际输出
"""
import asyncio
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from app.services.image_service import ImageService
from app.db.session import AsyncSessionLocal
from sqlalchemy import select
from app.models.user import User

async def test_bitmap_api():
    """模拟 API 调用"""
    async with AsyncSessionLocal() as db:
        # 查找用户 ID 4（有图片的用户）
        stmt = select(User).where(User.id == 4)
        result = await db.execute(stmt)
        user = result.scalar_one_or_none()

        if not user:
            print("User 4 not found")
            return

        # 获取用户的图片列表
        images = await ImageService.get_user_images(db, user.id)

        if not images:
            print("No images found for user 4")
            return

        print(f"Found {len(images)} images for user {user.id}")

        # 测试第一张图片的 bitmap
        image = images[0]
        print(f"\nTesting image ID: {image.id}")
        print(f"File path: {image.file_path}")

        try:
            bitmap_bytes = await ImageService.get_image_data(db, user.id, image.id)
            print(f"SUCCESS: Bitmap generated")
            print(f"  Size: {len(bitmap_bytes)} bytes")
            print(f"  Expected: 2808 bytes")

            if len(bitmap_bytes) == 2808:
                print("PASS: Size is correct!")
            else:
                print(f"FAIL: Size mismatch! Difference: {len(bitmap_bytes) - 2808} bytes")

        except Exception as e:
            print(f"ERROR: {e}")
            import traceback
            traceback.print_exc()

if __name__ == "__main__":
    asyncio.run(test_bitmap_api())
