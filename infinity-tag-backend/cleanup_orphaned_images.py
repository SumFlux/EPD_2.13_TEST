"""
清理数据库中的旧图片记录
"""
import asyncio
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession
from sqlalchemy.orm import sessionmaker
from sqlalchemy import select, delete
from app.models.image import CustomImage
from app.config import settings

async def cleanup_orphaned_images():
    """删除文件不存在的图片记录"""
    # 创建数据库连接
    engine = create_async_engine(settings.DATABASE_URL, echo=False)
    async_session = sessionmaker(engine, class_=AsyncSession, expire_on_commit=False)

    async with async_session() as session:
        # 查询所有图片记录
        stmt = select(CustomImage)
        result = await session.execute(stmt)
        images = result.scalars().all()

        print(f"Found {len(images)} image records in database")

        deleted_count = 0
        for img in images:
            # 检查文件是否存在
            file_path = Path(settings.ASSETS_DIR) / img.file_path
            if not file_path.exists():
                print(f"Deleting orphaned record: ID={img.id}, path={img.file_path}")
                await session.delete(img)
                deleted_count += 1

        if deleted_count > 0:
            await session.commit()
            print(f"\nDeleted {deleted_count} orphaned records")
        else:
            print("\nNo orphaned records found")

if __name__ == "__main__":
    asyncio.run(cleanup_orphaned_images())
