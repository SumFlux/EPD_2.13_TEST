from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.ext.asyncio import AsyncSession
from app.api import deps
from app.services.auth_service import AuthService
from app.schemas.auth import DeviceActivateRequest, LoginRequest, ActivateResponse, LoginResponse, UserResponse

router = APIRouter()

@router.post("/activate", response_model=ActivateResponse)
async def activate_device(
    request: DeviceActivateRequest,
    db: AsyncSession = Depends(deps.get_db)
):
    """
    设备激活/注册接口
    返回: Token + Device Secret (仅一次)
    """
    return await AuthService.activate_device(
        db,
        request.device_id,
        request.password
    )

@router.post("/login", response_model=LoginResponse)
async def login(
    request: LoginRequest,
    db: AsyncSession = Depends(deps.get_db)
):
    """
    设备登录接口
    返回: 仅 Token
    """
    return await AuthService.login(
        db,
        request.device_id,
        request.password
    )

@router.get("/me", response_model=UserResponse)
async def read_users_me(
    current_user = Depends(deps.get_current_user)
):
    """
    获取当前用户信息
    """
    return current_user
