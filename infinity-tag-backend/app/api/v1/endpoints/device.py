"""
设备 API 端点 - ESP32 查询接口
"""
from fastapi import APIRouter, Depends, Query
from sqlalchemy.ext.asyncio import AsyncSession
from app.api import deps
from app.services.device_service import DeviceService
from app.schemas.device import DeviceStatusResponse

router = APIRouter()


@router.get("/status", response_model=DeviceStatusResponse)
async def get_device_status(
    code: str = Query(..., min_length=6, max_length=6, description="6位设备码"),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    ESP32 查询设备激活状态
    无需认证，公开接口
    """
    service = DeviceService(db)
    return await service.get_device_status(code)
