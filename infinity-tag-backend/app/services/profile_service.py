from typing import Optional
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from app.models.user import UserProfile
from app.schemas.profile import ProfileCreate, ProfileUpdate
from app.core.bazi_calculator import BaziCalculator

class ProfileService:

    @staticmethod
    async def get_profile(db: AsyncSession, user_id: int) -> Optional[UserProfile]:
        stmt = select(UserProfile).where(UserProfile.user_id == user_id)
        result = await db.execute(stmt)
        return result.scalars().first()

    @staticmethod
    async def create_or_update_profile(
        db: AsyncSession,
        user_id: int,
        profile_in: ProfileCreate
    ) -> UserProfile:
        # 1. 计算八字
        bazi = BaziCalculator.calculate(profile_in.birth_datetime)

        # 2. 查找现有档案
        stmt = select(UserProfile).where(UserProfile.user_id == user_id)
        result = await db.execute(stmt)
        existing_profile = result.scalars().first()

        if existing_profile:
            # 更新
            existing_profile.birth_datetime = profile_in.birth_datetime
            existing_profile.birth_place = profile_in.birth_place
            existing_profile.profession = profile_in.profession
            existing_profile.focus_areas = profile_in.focus_areas
            existing_profile.bazi_year = bazi["year"]
            existing_profile.bazi_month = bazi["month"]
            existing_profile.bazi_day = bazi["day"]
            existing_profile.bazi_hour = bazi["hour"]

            await db.commit()
            await db.refresh(existing_profile)
            return existing_profile
        else:
            # 创建
            new_profile = UserProfile(
                user_id=user_id,
                birth_datetime=profile_in.birth_datetime,
                birth_place=profile_in.birth_place,
                profession=profile_in.profession,
                focus_areas=profile_in.focus_areas,
                bazi_year=bazi["year"],
                bazi_month=bazi["month"],
                bazi_day=bazi["day"],
                bazi_hour=bazi["hour"]
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
        stmt = select(UserProfile).where(UserProfile.user_id == user_id)
        result = await db.execute(stmt)
        profile = result.scalars().first()

        if not profile:
            return None

        update_data = profile_in.model_dump(exclude_unset=True)

        # 如果更新了出生时间，需要重新计算八字
        if "birth_datetime" in update_data:
            bazi = BaziCalculator.calculate(update_data["birth_datetime"])
            profile.bazi_year = bazi["year"]
            profile.bazi_month = bazi["month"]
            profile.bazi_day = bazi["day"]
            profile.bazi_hour = bazi["hour"]

        for field, value in update_data.items():
            setattr(profile, field, value)

        await db.commit()
        await db.refresh(profile)
        return profile
