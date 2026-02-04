import os
import uuid
import aiofiles
from typing import List
from fastapi import UploadFile, HTTPException
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, func
from fastapi.concurrency import run_in_threadpool
from app.models.image import CustomImage
from app.models.user import User
from app.schemas.image import ImageReorder, ImageProcessOptions
from app.config import settings
from app.services.image_processing import ImageProcessor

class ImageService:
    """
    图片管理服务：处理上传、存储、数据库记录
    """

    MAX_IMAGES_PER_USER = 5

    @staticmethod
    def _get_user_image_dir(user_id: int) -> str:
        """获取用户图片存储目录"""
        # 使用 settings.ASSETS_DIR/custom_images/{user_id}
        path = os.path.join(settings.ASSETS_DIR, "custom_images", str(user_id))
        os.makedirs(path, exist_ok=True)
        return path

    @staticmethod
    async def upload_image(
        db: AsyncSession,
        user: User,
        file: UploadFile,
        options: ImageProcessOptions = None
    ) -> CustomImage:
        """
        上传并处理图片
        """
        # 1. 检查数量限制
        stmt = select(func.count()).select_from(CustomImage).where(CustomImage.user_id == user.id)
        count = await db.scalar(stmt)
        if count >= ImageService.MAX_IMAGES_PER_USER:
            raise HTTPException(status_code=400, detail=f"最多只能上传 {ImageService.MAX_IMAGES_PER_USER} 张图片")

        # 2. 验证文件类型
        if not file.content_type.startswith("image/"):
            raise HTTPException(status_code=400, detail="仅支持图片文件")

        # 3. 读取并处理图片
        content = await file.read()
        try:
            # 这里的 process_image 是 CPU 密集型操作，使用 run_in_threadpool 放入线程池
            processed_bytes = await run_in_threadpool(ImageProcessor.process_image, content, options)
        except Exception as e:
            raise HTTPException(status_code=400, detail=f"图片处理失败: {str(e)}")

        # 4. 保存文件
        file_id = uuid.uuid4().hex
        filename = f"{file_id}.png"
        user_dir = ImageService._get_user_image_dir(user.id)
        file_path = os.path.join(user_dir, filename)

        async with aiofiles.open(file_path, 'wb') as f:
            await f.write(processed_bytes)

        # 5. 创建数据库记录
        # 计算新的 order (放在最后)
        max_order_stmt = select(func.max(CustomImage.display_order)).where(CustomImage.user_id == user.id)
        max_order = await db.scalar(max_order_stmt)
        new_order = (max_order or 0) + 1

        # 保存相对路径，方便迁移和 URL 生成
        relative_path = os.path.join("custom_images", str(user.id), filename)

        db_image = CustomImage(
            user_id=user.id,
            file_path=relative_path,
            display_order=new_order
        )
        db.add(db_image)
        await db.commit()
        await db.refresh(db_image)

        return db_image

    @staticmethod
    async def get_user_images(db: AsyncSession, user_id: int) -> List[CustomImage]:
        """获取用户图片列表"""
        stmt = select(CustomImage).where(CustomImage.user_id == user_id).order_by(CustomImage.display_order)
        result = await db.execute(stmt)
        return result.scalars().all()

    @staticmethod
    async def delete_image(db: AsyncSession, user_id: int, image_id: int):
        """删除图片"""
        stmt = select(CustomImage).where(CustomImage.user_id == user_id, CustomImage.id == image_id)
        image = await db.scalar(stmt)

        if not image:
            raise HTTPException(status_code=404, detail="图片不存在")

        # 删除物理文件
        full_path = os.path.join(settings.ASSETS_DIR, image.file_path)
        if os.path.exists(full_path):
            os.remove(full_path)

        # 删除数据库记录
        await db.delete(image)
        await db.commit()

    @staticmethod
    async def reorder_images(db: AsyncSession, user_id: int, orders: List[ImageReorder]):
        """重新排序"""
        # 简单的实现：遍历更新。如果列表很长需要优化，但这里最多5张。
        stmt = select(CustomImage).where(CustomImage.user_id == user_id)
        result = await db.execute(stmt)
        images = {img.id: img for img in result.scalars().all()}

        for item in orders:
            if item.id in images:
                images[item.id].display_order = item.new_order

        await db.commit()

    @staticmethod
    async def get_image_data(db: AsyncSession, user_id: int, image_id: int) -> bytes:
        """获取图片二进制数据 (用于 EPD)"""
        stmt = select(CustomImage).where(CustomImage.user_id == user_id, CustomImage.id == image_id)
        image = await db.scalar(stmt)

        if not image:
            raise HTTPException(status_code=404, detail="图片不存在")

        full_path = os.path.join(settings.ASSETS_DIR, image.file_path)
        if not os.path.exists(full_path):
            raise HTTPException(status_code=404, detail="图片文件丢失")

        # 增加查看计数
        image.view_count += 1
        await db.commit()

        return await run_in_threadpool(ImageProcessor.get_epd_bitmap, full_path)
