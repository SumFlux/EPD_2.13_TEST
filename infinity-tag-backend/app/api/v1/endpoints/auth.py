"""
认证 API 端点

提供设备激活、密码设置、用户登录等认证相关接口。
支持新的设备码+初始密码激活流程。
"""
from fastapi import APIRouter, Depends
from sqlalchemy.ext.asyncio import AsyncSession
from app.api import deps
from app.services.auth_service import AuthService
from app.schemas.auth import (
    DeviceActivateRequest,
    ActivateResponse,
    SetPasswordRequest,
    SetPasswordResponse,
    LoginRequest,
    LoginResponse,
    UserResponse
)

router = APIRouter()


@router.post("/activate", response_model=ActivateResponse)
async def activate_device(
    request: DeviceActivateRequest,
    db: AsyncSession = Depends(deps.get_db)
):
    """
    设备激活接口（新流程）
    验证设备码+初始密码，返回临时token用于设置密码
    """
    service = AuthService(db)
    return await service.activate_device(request)


@router.post("/set-password", response_model=SetPasswordResponse)
async def set_password(
    request: SetPasswordRequest,
    user_id: int = Depends(deps.get_temp_token_user_id),
    db: AsyncSession = Depends(deps.get_db)
):
    """首次设置用户密码（激活后必须调用）"""
    service = AuthService(db)
    return await service.set_password(user_id, request)


@router.post("/login", response_model=LoginResponse)
async def login(
    request: LoginRequest,
    db: AsyncSession = Depends(deps.get_db)
):
    """
    用户登录接口
    使用设备码+用户密码登录
    """
    service = AuthService(db)
    return await service.login(request)


@router.get("/me", response_model=UserResponse)
async def read_users_me(
    current_user=Depends(deps.get_current_user)
):
    """
    获取当前用户信息
    """
    return current_user
