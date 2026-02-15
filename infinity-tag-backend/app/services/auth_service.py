"""
认证服务 - 设备即用户架构
"""
from datetime import datetime, timezone, timedelta
from fastapi import HTTPException, status
from sqlalchemy.ext.asyncio import AsyncSession
from app.repositories.user_repo import UserRepository
from app.core.security import create_access_token, verify_password, get_password_hash
from app.schemas.auth import (
    DeviceActivateRequest,
    ActivateResponse,
    SetPasswordRequest,
    SetPasswordResponse,
    LoginRequest,
    LoginResponse
)
from app.models.user import DeviceStatus
import secrets


class AuthService:
    def __init__(self, db: AsyncSession):
        self.user_repo = UserRepository(db)
        self.db = db

    async def activate_device(self, request: DeviceActivateRequest) -> ActivateResponse:
        """
        激活流程：验证设备码+初始密码
        返回临时token，用于设置用户密码
        """
        # 1. 查找用户（通过 device_code）
        user = await self.user_repo.get_by_device_code(request.device_code)
        if not user:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="无效的设备码"
            )

        # 2. 检查设备状态
        if user.status == DeviceStatus.ACTIVATED.value:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="该设备已被激活"
            )

        if user.status == DeviceStatus.DISABLED.value:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="该设备已被禁用"
            )

        # 3. 验证初始密码
        if not verify_password(request.init_password, user.init_password_hash):
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="初始密码错误"
            )

        # 4. 检查是否已经设置过密码
        if user.password_set:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="该设备已被激活"
            )

        # 5. 生成临时token（短期有效，仅用于设置密码）
        temp_token = create_access_token(
            subject=f"temp:{user.id}",
            expires_delta=timedelta(minutes=30)
        )

        return ActivateResponse(
            success=True,
            requires_password_setup=True,
            temp_token=temp_token,
            device_code=request.device_code
        )

    async def set_password(
        self, user_id: int, request: SetPasswordRequest
    ) -> SetPasswordResponse:
        """
        首次设置用户密码（激活后必须调用）
        """
        user = await self.user_repo.get_by_id(user_id)
        if not user:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="用户不存在"
            )

        if user.password_set:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="密码已设置，请使用登录接口"
            )

        # 设置密码并更新状态
        user.password_hash = get_password_hash(request.new_password)
        user.password_set = True
        user.status = DeviceStatus.ACTIVATED.value
        user.activated_at = datetime.now(timezone.utc)
        await self.db.commit()
        await self.db.refresh(user)

        # 生成正式token
        access_token = create_access_token(subject=user.id)

        return SetPasswordResponse(
            access_token=access_token,
            device_id=user.device_code
        )

    async def login(self, request: LoginRequest) -> LoginResponse:
        """
        用户登录（设备码+用户密码）
        """
        user = await self.user_repo.get_by_device_code(request.device_code)
        if not user:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="设备码或密码错误"
            )

        if not user.password_set or not user.password_hash:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="请先完成激活并设置密码"
            )

        if not verify_password(request.password, user.password_hash):
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="设备码或密码错误"
            )

        # 检查设备状态
        if user.status == DeviceStatus.DISABLED.value:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="该设备已被禁用"
            )

        await self.user_repo.update_login_time(user)
        access_token = create_access_token(subject=user.id)

        return LoginResponse(access_token=access_token)
