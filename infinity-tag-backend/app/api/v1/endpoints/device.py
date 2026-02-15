"""
设备 API 端点 - ESP32 查询接口（设备即用户架构）
"""
from fastapi import APIRouter, Depends, Query, HTTPException, status
from sqlalchemy.ext.asyncio import AsyncSession
from app.api import deps
from app.repositories.user_repo import UserRepository
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
    user_repo = UserRepository(db)
    user = await user_repo.get_by_device_code(code)

    if not user:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="设备不存在"
        )

    return DeviceStatusResponse(
        device_code=user.device_code,
        status=user.status,
        activated_at=user.activated_at
    )
