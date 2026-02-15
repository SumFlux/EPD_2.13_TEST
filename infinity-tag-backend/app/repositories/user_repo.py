from typing import Optional, List
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from app.models.user import User
from datetime import datetime, timezone

class UserRepository:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def get_by_id(self, user_id: int) -> Optional[User]:
        stmt = select(User).where(User.id == user_id)
        result = await self.session.execute(stmt)
        return result.scalar_one_or_none()

    async def get_by_device_code(self, device_code: str) -> Optional[User]:
        """按 device_code 查询用户"""
        stmt = select(User).where(User.device_code == device_code)
        result = await self.session.execute(stmt)
        return result.scalar_one_or_none()

    async def get_by_uuid(self, uuid: str) -> Optional[User]:
        """按 uuid 查询用户"""
        stmt = select(User).where(User.uuid == uuid)
        result = await self.session.execute(stmt)
        return result.scalar_one_or_none()

    async def get_by_status(self, status: str, limit: int = 100, offset: int = 0) -> List[User]:
        """按 status 过滤用户"""
        stmt = select(User).where(User.status == status).limit(limit).offset(offset)
        result = await self.session.execute(stmt)
        return result.scalars().all()

    async def create_user(self, user_data: dict) -> User:
        user = User(**user_data)
        self.session.add(user)
        await self.session.commit()
        await self.session.refresh(user)
        return user

    async def update_login_time(self, user: User) -> None:
        user.last_login_at = datetime.now(timezone.utc)
        await self.session.commit()
