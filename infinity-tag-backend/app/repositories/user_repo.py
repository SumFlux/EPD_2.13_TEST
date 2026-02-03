from typing import Optional
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from app.models.user import User
from datetime import datetime, timezone

class UserRepository:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def get_by_device_id(self, device_id: str) -> Optional[User]:
        stmt = select(User).where(User.device_id == device_id)
        result = await self.session.execute(stmt)
        return result.scalar_one_or_none()

    async def create_user(self, user_data: dict) -> User:
        user = User(**user_data)
        self.session.add(user)
        await self.session.commit()
        await self.session.refresh(user)
        return user

    async def update_login_time(self, user: User) -> None:
        user.last_login_at = datetime.now(timezone.utc)
        await self.session.commit()
