from fastapi import APIRouter, Depends
from sqlalchemy.ext.asyncio import AsyncSession
from app.api import deps
from app.services.auth_service import AuthService
from app.schemas.user import DeviceActivateRequest, TokenResponse
from app.schemas.common import ResponseBase

router = APIRouter()


@router.post("/activate", response_model=ResponseBase[TokenResponse])
async def activate_device(
    data: DeviceActivateRequest,
    db: AsyncSession = Depends(deps.get_db)
):
    """
    设备激活/登录
    通过 6 位设备短码获取 Access Token
    """
    token = await AuthService.activate_device(db, data)
    return ResponseBase(data=token)
