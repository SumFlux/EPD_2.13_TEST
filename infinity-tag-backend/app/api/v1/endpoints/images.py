from typing import List
from fastapi import APIRouter, Depends, UploadFile, File, Form, HTTPException, Response
from fastapi.concurrency import run_in_threadpool
from sqlalchemy.ext.asyncio import AsyncSession
from app.api import deps
from app.models.user import User
from app.schemas.image import ImageOut, ImageReorder, ImageProcessOptions
from app.services.image_service import ImageService
from app.services.image_processing import ImageProcessor
import os

router = APIRouter()

@router.post("/preview")
async def preview_image(
    file: UploadFile = File(...),
    options: str = Form(default='{}')
):
    """
    预览图片处理效果 (不保存)
    前端可以发送原图 + 参数，实时查看效果
    """
    if file.content_type and not file.content_type.startswith("image/"):
        raise HTTPException(status_code=400, detail="仅支持图片文件")

    try:
        options_obj = ImageProcessOptions.model_validate_json(options)
    except Exception as e:
        raise HTTPException(status_code=400, detail=f"参数解析失败: {str(e)}")

    content = await file.read()
    try:
        # 修复：使用 run_in_threadpool
        processed_bytes = await run_in_threadpool(
            ImageProcessor.process_image, content, options_obj
        )
    except Exception as e:
        raise HTTPException(status_code=400, detail=f"处理失败: {str(e)}")

    return Response(content=processed_bytes, media_type="image/png")

@router.post("/", response_model=ImageOut)
async def upload_image(
    file: UploadFile = File(...),
    options: str = Form(default='{}'),
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    上传图片 (支持裁剪、二值化等参数)
    """
    try:
        options_obj = ImageProcessOptions.model_validate_json(options)
    except Exception as e:
        raise HTTPException(status_code=400, detail=f"参数解析失败: {str(e)}")

    image = await ImageService.upload_image(db, current_user, file, options_obj)

    # 构造完整的 URL
    url = f"/assets/{image.file_path.replace(os.sep, '/')}"

    return ImageOut(
        id=image.id,
        display_order=image.display_order,
        view_count=image.view_count,
        created_at=image.created_at,
        url=url
    )

@router.get("/", response_model=List[ImageOut])
async def get_images(
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    获取用户的所有自定义图片
    """
    images = await ImageService.get_user_images(db, current_user.id)

    # 转换 URL
    result = []
    for img in images:
        url = f"/assets/{img.file_path.replace(os.sep, '/')}"
        result.append(ImageOut(
            id=img.id,
            display_order=img.display_order,
            view_count=img.view_count,
            created_at=img.created_at,
            url=url
        ))
    return result

@router.delete("/{image_id}")
async def delete_image(
    image_id: int,
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    删除图片
    """
    await ImageService.delete_image(db, current_user.id, image_id)
    return {"success": True}

@router.put("/reorder")
async def reorder_images(
    orders: List[ImageReorder],
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    重新排序
    """
    await ImageService.reorder_images(db, current_user.id, orders)
    return {"success": True}

@router.get("/{image_id}/bitmap")
async def get_image_bitmap(
    image_id: int,
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    获取图片的原始位图数据 (供 ESP32 下载)
    """
    bitmap_bytes = await ImageService.get_image_data(db, current_user.id, image_id)
    return Response(content=bitmap_bytes, media_type="application/octet-stream")
