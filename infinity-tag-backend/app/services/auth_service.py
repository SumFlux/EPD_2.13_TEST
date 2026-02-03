"""
认证服务
处理设备激活、登录与 Token 签发
"""
from datetime import datetime
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from fastapi import HTTPException, status

from app.models.user import User
from app.schemas.user import DeviceActivateRequest, TokenResponse
from app.core.security import create_access_token
from app.config import settings


class AuthService:
    """认证业务逻辑"""

    @staticmethod
    async def activate_device(db: AsyncSession, data: DeviceActivateRequest) -> TokenResponse:
        """
        激活或登录设备
        :param db: 数据库会话
        :param data: 激活参数 (device_code, device_uuid)
        :return: Token响应
        """
        # 1. 查询设备是否存在
        stmt = select(User).where(User.device_code == data.device_code)
        result = await db.execute(stmt)
        user = result.scalar_one_or_none()

        if not user:
            # 2. 不存在则注册新设备
            user = User(
                device_code=data.device_code,
                device_uuid=data.device_uuid,
                is_active=True,
                last_login_at=datetime.now().isoformat()
            )
            db.add(user)
            await db.commit()
            await db.refresh(user)
        else:
            # 3. 存在则验证与更新
            # 安全检查: 如果数据库中已绑定 UUID，且请求中也带了 UUID，则必须一致
            if user.device_uuid and data.device_uuid:
                if user.device_uuid != data.device_uuid:
                    raise HTTPException(
                        status_code=status.HTTP_403_FORBIDDEN,
                        detail="设备硬件ID不匹配，拒绝访问"
                    )

            # 如果之前没绑定 UUID，现在绑定
            if not user.device_uuid and data.device_uuid:
                user.device_uuid = data.device_uuid

            # 更新最后登录时间
            user.last_login_at = datetime.now().isoformat()
            await db.commit()

        # 4. 签发 Token (Subject 使用 User ID)
        access_token = create_access_token(
            subject=user.id
        )

        return TokenResponse(
            access_token=access_token,
            expires_in=settings.JWT_EXPIRATION_HOURS * 3600
        )
