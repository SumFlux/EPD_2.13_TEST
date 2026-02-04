from fastapi import APIRouter, Depends, HTTPException, Response
from fastapi.concurrency import run_in_threadpool
from sqlalchemy.ext.asyncio import AsyncSession
from app.api import deps
from app.services.almanac_service import AlmanacService
from app.services.renderer_service import RendererService
from app.models.user import User
from datetime import date

router = APIRouter()

@router.get("/preview")
async def preview_almanac(
    target_date: date = date.today(),
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    预览今日黄历渲染效果 (PNG)
    """
    # 1. 获取黄历
    almanac = await AlmanacService.get_almanac_by_date(db, current_user.id, target_date)

    if not almanac:
        # 简单起见，如果不存在则报错。实际可以尝试自动生成。
        # 这里为了演示，假设前端会先调 /almanac/generate
        raise HTTPException(status_code=404, detail="请先生成当日黄历 (Call /api/v1/almanac/generate first)")

    # 2. 渲染 (运行在线程池中，避免阻塞主循环)
    png_bytes = await run_in_threadpool(RendererService.render_almanac_card, almanac)

    return Response(content=png_bytes, media_type="image/png")

@router.get("/bitmap")
async def get_almanac_bitmap(
    target_date: date = date.today(),
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    获取 ESP32 用的原始位图数据 (Binary)
    """
    almanac = await AlmanacService.get_almanac_by_date(db, current_user.id, target_date)
    if not almanac:
        raise HTTPException(status_code=404, detail="Almanac not found")

    # 分步执行渲染和转换，均放入线程池
    png_bytes = await run_in_threadpool(RendererService.render_almanac_card, almanac)
    bitmap_bytes = await run_in_threadpool(RendererService.convert_to_epd_bitmap, png_bytes)

    return Response(content=bitmap_bytes, media_type="application/octet-stream")
