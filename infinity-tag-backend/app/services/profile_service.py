from typing import Optional, Dict, Any, List
from datetime import datetime
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from app.models.user import UserProfile
from app.schemas.profile import ProfileCreate, ProfileUpdate
from app.utils.lunar import LunarUtils

class ProfileService:
    """用户档案服务"""

    @staticmethod
    async def get_profile(db: AsyncSession, user_id: int) -> Optional[UserProfile]:
        """获取用户档案"""
        result = await db.execute(select(UserProfile).where(UserProfile.user_id == user_id))
        return result.scalars().first()

    @staticmethod
    async def create_or_update_profile(
        db: AsyncSession,
        user_id: int,
        profile_in: ProfileCreate
    ) -> UserProfile:
        """创建或更新用户档案 (自动计算八字)"""

        # 1. 计算八字
        # 注意：profile_in.birth_datetime 是 datetime 对象
        birth_dt = profile_in.birth_datetime

        bazi_dict = LunarUtils.get_ba_zi(
            year=birth_dt.year,
            month=birth_dt.month,
            day=birth_dt.day,
            hour=birth_dt.hour,
            minute=birth_dt.minute
        )

        # 2. 检查是否存在
        existing_profile = await ProfileService.get_profile(db, user_id)

        if existing_profile:
            # 更新
            existing_profile.birth_datetime = birth_dt
            existing_profile.birth_place = profile_in.birth_place
            existing_profile.profession = profile_in.profession
            existing_profile.focus_areas = profile_in.focus_areas
            existing_profile.bazi_year = bazi_dict['year']
            existing_profile.bazi_month = bazi_dict['month']
            existing_profile.bazi_day = bazi_dict['day']
            existing_profile.bazi_hour = bazi_dict['hour']

            await db.commit()
            await db.refresh(existing_profile)
            return existing_profile
        else:
            # 创建
            new_profile = UserProfile(
                user_id=user_id,
                birth_datetime=birth_dt,
                birth_place=profile_in.birth_place,
                profession=profile_in.profession,
                focus_areas=profile_in.focus_areas,
                bazi_year=bazi_dict['year'],
                bazi_month=bazi_dict['month'],
                bazi_day=bazi_dict['day'],
                bazi_hour=bazi_dict['hour']
            )
            db.add(new_profile)
            await db.commit()
            await db.refresh(new_profile)
            return new_profile

    @staticmethod
    async def update_profile_partial(
        db: AsyncSession,
        user_id: int,
        profile_in: ProfileUpdate
    ) -> Optional[UserProfile]:
        """部分更新档案"""
        profile = await ProfileService.get_profile(db, user_id)
        if not profile:
            return None

        update_data = profile_in.model_dump(exclude_unset=True)

        # 如果更新了出生时间，需要重新计算八字
        if "birth_datetime" in update_data:
            birth_dt = update_data["birth_datetime"]
            bazi_dict = LunarUtils.get_ba_zi(
                year=birth_dt.year,
                month=birth_dt.month,
                day=birth_dt.day,
                hour=birth_dt.hour,
                minute=birth_dt.minute
            )
            profile.bazi_year = bazi_dict['year']
            profile.bazi_month = bazi_dict['month']
            profile.bazi_day = bazi_dict['day']
            profile.bazi_hour = bazi_dict['hour']

        for field, value in update_data.items():
            setattr(profile, field, value)

        await db.commit()
        await db.refresh(profile)
        return profile
