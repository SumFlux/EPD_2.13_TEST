"""
管理员业务逻辑服务
"""
from datetime import timedelta
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, func
from fastapi import HTTPException, status
from app.config import settings
from app.core.security import verify_password, create_access_token
from app.models.user import User
from app.repositories.device_repository import DeviceRepository
from app.schemas.admin import (
    AdminLoginRequest,
    AdminLoginResponse,
    AdminUserResponse,
    AdminUserListResponse,
    AdminUserDetailResponse,
    AdminStatsResponse
)


class AdminService:
    def __init__(self, db: AsyncSession):
        self.db = db
        self.device_repo = DeviceRepository(db)

    async def login(self, request: AdminLoginRequest) -> AdminLoginResponse:
        """管理员登录"""
        if request.username != settings.ADMIN_USERNAME:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="用户名或密码错误"
            )

        if not verify_password(request.password, settings.ADMIN_PASSWORD_HASH):
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="用户名或密码错误"
            )

        # 生成管理员 token（带 admin 标识）
        expires_delta = timedelta(hours=settings.ADMIN_JWT_EXPIRATION_HOURS)
        access_token = create_access_token(
            subject=f"admin:{request.username}",
            expires_delta=expires_delta
        )

        return AdminLoginResponse(
            access_token=access_token,
            expires_in=settings.ADMIN_JWT_EXPIRATION_HOURS * 3600
        )

    async def get_user_list(
        self,
        page: int = 1,
        page_size: int = 20
    ) -> AdminUserListResponse:
        """获取用户列表"""
        stmt = select(User)

        # 计算总数
        count_stmt = select(func.count(User.id))
        total_result = await self.db.execute(count_stmt)
        total = total_result.scalar() or 0

        # 分页
        stmt = stmt.order_by(User.created_at.desc())
        stmt = stmt.offset((page - 1) * page_size).limit(page_size)

        result = await self.db.execute(stmt)
        users = result.scalars().all()

        return AdminUserListResponse(
            total=total,
            page=page,
            page_size=page_size,
            users=[AdminUserResponse.model_validate(u) for u in users]
        )

    async def get_user_detail(self, user_id: int) -> AdminUserDetailResponse:
        """获取用户详情（含关联设备）"""
        stmt = select(User).where(User.id == user_id)
        result = await self.db.execute(stmt)
        user = result.scalar_one_or_none()

        if not user:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="用户不存在"
            )

        # 获取关联设备
        device = await self.device_repo.get_by_device_code(user.device_id)

        return AdminUserDetailResponse(
            id=user.id,
            device_id=user.device_id,
            password_set=user.password_set,
            activated_at=user.activated_at,
            last_login_at=user.last_login_at,
            created_at=user.created_at,
            device_code=device.device_code if device else None,
            device_status=device.status if device else None
        )

    async def disable_user(self, user_id: int) -> AdminUserResponse:
        """禁用用户（通过禁用其关联设备）"""
        stmt = select(User).where(User.id == user_id)
        result = await self.db.execute(stmt)
        user = result.scalar_one_or_none()

        if not user:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="用户不存在"
            )

        # 禁用关联设备
        device = await self.device_repo.get_by_device_code(user.device_id)
        if device:
            await self.device_repo.disable(device)

        return AdminUserResponse.model_validate(user)

    async def get_stats(self) -> AdminStatsResponse:
        """获取统计数据"""
        # 设备统计
        device_stats = await self.device_repo.get_stats()

        # 用户统计
        total_users_stmt = select(func.count(User.id))
        total_users = (await self.db.execute(total_users_stmt)).scalar() or 0

        users_with_password_stmt = select(func.count(User.id)).where(
            User.password_set == True
        )
        users_with_password = (
            await self.db.execute(users_with_password_stmt)
        ).scalar() or 0

        return AdminStatsResponse(
            total_devices=device_stats["total_devices"],
            pending_devices=device_stats["pending_devices"],
            activated_devices=device_stats["activated_devices"],
            disabled_devices=device_stats["disabled_devices"],
            total_users=total_users,
            users_with_password=users_with_password
        )
