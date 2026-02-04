"""
用户服务
处理用户档案 (Profile) 管理
"""
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select

from app.models.user import UserProfile
from app.schemas.user import UserProfileCreate


class UserService:
    """用户业务逻辑"""

    @staticmethod
    async def get_profile(db: AsyncSession, user_id: int) -> UserProfile:
        """获取用户档案"""
        stmt = select(UserProfile).where(UserProfile.user_id == user_id)
        result = await db.execute(stmt)
        return result.scalar_one_or_none()

    @staticmethod
    async def create_or_update_profile(
        db: AsyncSession,
        user_id: int,
        data: UserProfileCreate
    ) -> UserProfile:
        """创建或更新档案"""
        # 1. 检查是否存在
        stmt = select(UserProfile).where(UserProfile.user_id == user_id)
        result = await db.execute(stmt)
        profile = result.scalar_one_or_none()

        if profile:
            # 2. 更新 (使用 setattr 动态更新字段)
            update_data = data.model_dump(exclude_unset=True)
            for key, value in update_data.items():
                setattr(profile, key, value)
        else:
            # 3. 创建
            profile = UserProfile(user_id=user_id, **data.model_dump())
            db.add(profile)

        await db.commit()
        await db.refresh(profile)
        return profile
