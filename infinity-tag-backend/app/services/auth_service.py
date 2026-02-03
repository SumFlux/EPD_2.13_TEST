from typing import Optional, Dict, Any, Union
from datetime import datetime
from sqlalchemy.future import select
from sqlalchemy.ext.asyncio import AsyncSession
from fastapi import HTTPException, status
from app.models.user import User
from app.core.security import verify_password, get_password_hash, create_access_token
from app.utils.device_code_gen import generate_device_id
from app.schemas.auth import ActivateResponse, LoginResponse
import uuid

class AuthService:
    """认证服务"""

    @staticmethod
    async def activate_device(
        db: AsyncSession,
        device_id: Optional[str],
        password: str
    ) -> ActivateResponse:
        """
        激活或注册设备
        如果 device_id 为空，则生成新的设备码
        """
        # 1. 验证或生成设备码
        if not device_id:
            device_id = generate_device_id()
            # 确保唯一性
            while True:
                existing = await db.execute(select(User).where(User.device_id == device_id))
                if not existing.scalars().first():
                    break
                device_id = generate_device_id()
        else:
            # 检查是否已存在
            result = await db.execute(select(User).where(User.device_id == device_id))
            if result.scalars().first():
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail="设备ID已存在，请直接登录"
                )

        # 2. 生成设备密钥 (HMAC Secret)
        device_secret = uuid.uuid4().hex

        # 3. 创建用户
        user = User(
            device_id=device_id,
            password_hash=get_password_hash(password),
            device_secret=device_secret,
            activated_at=datetime.utcnow(),
            last_login_at=datetime.utcnow()
        )
        db.add(user)
        await db.commit()
        await db.refresh(user)

        # 4. 生成 Token
        access_token = create_access_token(subject=user.id)

        # 5. 返回 Pydantic 模型，避免直接返回 ORM 对象
        return ActivateResponse(
            access_token=access_token,
            device_secret=device_secret,
            user_id=user.id,
            device_id=user.device_id
        )

    @staticmethod
    async def authenticate_user(
        db: AsyncSession,
        device_id: str,
        password: str
    ) -> Optional[User]:
        """验证用户凭据"""
        result = await db.execute(select(User).where(User.device_id == device_id))
        user = result.scalars().first()

        if not user:
            return None

        if not verify_password(password, user.password_hash):
            return None

        return user

    @staticmethod
    async def login(
        db: AsyncSession,
        device_id: str,
        password: str
    ) -> LoginResponse:
        """登录逻辑"""
        user = await AuthService.authenticate_user(db, device_id, password)
        if not user:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="设备ID或密码错误",
                headers={"WWW-Authenticate": "Bearer"},
            )

        # 更新最后登录时间
        user.last_login_at = datetime.utcnow()
        await db.commit()

        access_token = create_access_token(subject=user.id)

        # 登录仅返回 Token，不返回 device_secret
        return LoginResponse(
            access_token=access_token
        )
