from typing import Optional
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

        # 1. 根据输入计算八字
        # 如果是农历，需要先转换为公历
        year = profile_in.birth_year
        month = profile_in.birth_month
        day = profile_in.birth_day
        hour = profile_in.birth_hour if profile_in.birth_hour >= 0 else 12  # 未知时辰默认用午时

        if profile_in.is_lunar:
            # 农历转公历
            solar = LunarUtils.lunar_to_solar(year, month, day)
            year, month, day = solar['year'], solar['month'], solar['day']

        bazi_dict = LunarUtils.get_ba_zi(
            year=year,
            month=month,
            day=day,
            hour=hour,
            minute=0
        )

        # 2. 检查是否存在
        existing_profile = await ProfileService.get_profile(db, user_id)

        if existing_profile:
            # 更新
            existing_profile.nickname = profile_in.nickname
            existing_profile.gender = profile_in.gender
            existing_profile.birth_year = profile_in.birth_year
            existing_profile.birth_month = profile_in.birth_month
            existing_profile.birth_day = profile_in.birth_day
            existing_profile.birth_hour = profile_in.birth_hour
            existing_profile.is_lunar = profile_in.is_lunar
            existing_profile.occupation = profile_in.occupation
            existing_profile.notes = profile_in.notes
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
                nickname=profile_in.nickname,
                gender=profile_in.gender,
                birth_year=profile_in.birth_year,
                birth_month=profile_in.birth_month,
                birth_day=profile_in.birth_day,
                birth_hour=profile_in.birth_hour,
                is_lunar=profile_in.is_lunar,
                occupation=profile_in.occupation,
                notes=profile_in.notes,
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

        # 如果更新了出生信息，需要重新计算八字
        birth_fields = {'birth_year', 'birth_month', 'birth_day', 'birth_hour', 'is_lunar'}
        if birth_fields & set(update_data.keys()):
            # 合并现有值和更新值
            year = update_data.get('birth_year', profile.birth_year)
            month = update_data.get('birth_month', profile.birth_month)
            day = update_data.get('birth_day', profile.birth_day)
            hour = update_data.get('birth_hour', profile.birth_hour)
            is_lunar = update_data.get('is_lunar', profile.is_lunar)

            calc_hour = hour if hour >= 0 else 12

            if is_lunar:
                solar = LunarUtils.lunar_to_solar(year, month, day)
                year, month, day = solar['year'], solar['month'], solar['day']

            bazi_dict = LunarUtils.get_ba_zi(
                year=year,
                month=month,
                day=day,
                hour=calc_hour,
                minute=0
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
