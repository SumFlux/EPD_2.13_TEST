"""
管理员业务逻辑服务 - 设备即用户架构
"""
from datetime import timedelta
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, func
from fastapi import HTTPException, status
from app.config import settings
from app.core.security import verify_password, create_access_token, get_password_hash
from app.models.user import User, DeviceStatus
from app.repositories.user_repo import UserRepository
from app.schemas.admin import (
    AdminLoginRequest,
    AdminLoginResponse,
    AdminUserResponse,
    AdminUserListResponse,
    AdminUserDetailResponse,
    AdminStatsResponse,
    DeviceCreateRequest,
    DeviceCreateResponse,
    DeviceBatchImportRequest,
    DeviceBatchImportResponse
)
import secrets


class AdminService:
    def __init__(self, db: AsyncSession):
        self.db = db
        self.user_repo = UserRepository(db)

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

    async def create_device(self, request: DeviceCreateRequest) -> DeviceCreateResponse:
        """创建设备（实际创建 User 记录）"""
        # 检查 device_code 是否已存在
        existing = await self.user_repo.get_by_device_code(request.device_code)
        if existing:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="设备码已存在"
            )

        # 检查 uuid 是否已存在
        existing_uuid = await self.user_repo.get_by_uuid(request.uuid)
        if existing_uuid:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="UUID 已存在"
            )

        # 生成初始密码哈希和设备密钥
        init_password_hash = get_password_hash(request.init_password)
        device_secret = secrets.token_hex(32)

        # 创建用户记录
        user_data = {
            "device_code": request.device_code,
            "init_password_hash": init_password_hash,
            "uuid": request.uuid,
            "status": DeviceStatus.PENDING.value,
            "device_secret": device_secret,
            "password_set": False
        }

        user = await self.user_repo.create_user(user_data)

        return DeviceCreateResponse(
            id=user.id,
            device_code=user.device_code,
            uuid=user.uuid,
            status=user.status,
            created_at=user.created_at
        )

    async def batch_import_devices(
        self, request: DeviceBatchImportRequest
    ) -> DeviceBatchImportResponse:
        """批量导入设备（批量创建 User 记录）"""
        success_count = 0
        failed_count = 0
        errors = []

        for device_data in request.devices:
            try:
                # 检查重复
                existing = await self.user_repo.get_by_device_code(device_data.device_code)
                if existing:
                    failed_count += 1
                    errors.append(f"设备码 {device_data.device_code} 已存在")
                    continue

                existing_uuid = await self.user_repo.get_by_uuid(device_data.uuid)
                if existing_uuid:
                    failed_count += 1
                    errors.append(f"UUID {device_data.uuid} 已存在")
                    continue

                # 创建用户
                init_password_hash = get_password_hash(device_data.init_password)
                device_secret = secrets.token_hex(32)

                user_data = {
                    "device_code": device_data.device_code,
                    "init_password_hash": init_password_hash,
                    "uuid": device_data.uuid,
                    "status": DeviceStatus.PENDING.value,
                    "device_secret": device_secret,
                    "password_set": False
                }

                await self.user_repo.create_user(user_data)
                success_count += 1

            except Exception as e:
                failed_count += 1
                errors.append(f"设备码 {device_data.device_code}: {str(e)}")

        return DeviceBatchImportResponse(
            success_count=success_count,
            failed_count=failed_count,
            errors=errors if errors else None
        )

    async def get_device_list(
        self,
        status_filter: str = None,
        page: int = 1,
        page_size: int = 20
    ) -> AdminUserListResponse:
        """获取设备列表（查询 User 表）"""
        stmt = select(User)

        # 状态过滤
        if status_filter:
            stmt = stmt.where(User.status == status_filter)

        # 计算总数
        count_stmt = select(func.count(User.id))
        if status_filter:
            count_stmt = count_stmt.where(User.status == status_filter)
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

    async def get_device_detail(self, device_code: str) -> AdminUserDetailResponse:
        """获取设备详情（通过 device_code 查询 User）"""
        user = await self.user_repo.get_by_device_code(device_code)

        if not user:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="设备不存在"
            )

        return AdminUserDetailResponse(
            id=user.id,
            device_code=user.device_code,
            uuid=user.uuid,
            status=user.status,
            password_set=user.password_set,
            activated_at=user.activated_at,
            last_login_at=user.last_login_at,
            created_at=user.created_at
        )

    async def disable_device(self, device_code: str) -> AdminUserResponse:
        """禁用设备（更新 User.status）"""
        user = await self.user_repo.get_by_device_code(device_code)

        if not user:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="设备不存在"
            )

        # 更新状态为禁用
        user.status = DeviceStatus.DISABLED.value
        await self.db.commit()
        await self.db.refresh(user)

        return AdminUserResponse.model_validate(user)

    async def get_stats(self) -> AdminStatsResponse:
        """获取统计数据（基于 User 表）"""
        # 总设备数
        total_devices_stmt = select(func.count(User.id))
        total_devices = (await self.db.execute(total_devices_stmt)).scalar() or 0

        # 待激活设备数
        pending_devices_stmt = select(func.count(User.id)).where(
            User.status == DeviceStatus.PENDING.value
        )
        pending_devices = (await self.db.execute(pending_devices_stmt)).scalar() or 0

        # 已激活设备数
        activated_devices_stmt = select(func.count(User.id)).where(
            User.status == DeviceStatus.ACTIVATED.value
        )
        activated_devices = (await self.db.execute(activated_devices_stmt)).scalar() or 0

        # 已禁用设备数
        disabled_devices_stmt = select(func.count(User.id)).where(
            User.status == DeviceStatus.DISABLED.value
        )
        disabled_devices = (await self.db.execute(disabled_devices_stmt)).scalar() or 0

        # 已设置密码的用户数
        users_with_password_stmt = select(func.count(User.id)).where(
            User.password_set == True
        )
        users_with_password = (
            await self.db.execute(users_with_password_stmt)
        ).scalar() or 0

        return AdminStatsResponse(
            total_devices=total_devices,
            pending_devices=pending_devices,
            activated_devices=activated_devices,
            disabled_devices=disabled_devices,
            total_users=total_devices,  # 设备即用户
            users_with_password=users_with_password
        )
