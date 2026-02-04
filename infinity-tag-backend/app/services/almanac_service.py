from typing import Optional, List
from datetime import date, datetime
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, and_
from app.models.almanac import AlmanacHistory
from app.models.user import UserProfile
from app.utils.lunar import LunarUtils
from app.core.ai_service import ai_service
from app.services.fortune_service import generate_personalized_fortune

class AlmanacService:
    """黄历服务"""

    @staticmethod
    async def get_almanac_by_date(
        db: AsyncSession,
        user_id: int,
        target_date: date
    ) -> Optional[AlmanacHistory]:
        """查询指定日期的黄历"""
        result = await db.execute(
            select(AlmanacHistory).where(
                and_(
                    AlmanacHistory.user_id == user_id,
                    AlmanacHistory.date == target_date
                )
            )
        )
        return result.scalars().first()

    @staticmethod
    async def get_history(
        db: AsyncSession,
        user_id: int,
        limit: int = 30
    ) -> List[AlmanacHistory]:
        """查询历史黄历"""
        result = await db.execute(
            select(AlmanacHistory)
            .where(AlmanacHistory.user_id == user_id)
            .order_by(AlmanacHistory.date.desc())
            .limit(limit)
        )
        return result.scalars().all()

    @staticmethod
    async def generate_almanac(
        db: AsyncSession,
        user_id: int,
        profile: UserProfile,
        target_date: date
    ) -> AlmanacHistory:
        """
        生成黄历 (核心逻辑)
        如果已存在则直接返回
        """
        # 1. 检查缓存
        cached = await AlmanacService.get_almanac_by_date(db, user_id, target_date)
        if cached:
            return cached

        # 2. 计算日历信息
        lunar_info = LunarUtils.solar_to_lunar(target_date)
        # 获取当日干支 (需要准确的日柱)
        bazi_today = LunarUtils.get_ba_zi(target_date.year, target_date.month, target_date.day)

        # 3. 基于十神关系生成个性化运势
        fortune = generate_personalized_fortune(
            user_bazi_day=profile.bazi_day or "甲子",
            today_gan=bazi_today['day'],
            profession=profile.occupation,
            focus_areas=None  # notes 是文本备注，不再是列表
        )

        favorable = fortune['favorable']
        unfavorable = fortune['unfavorable']
        lucky_direction = fortune['lucky_direction']
        lucky_item = fortune['lucky_item']
        energy_level = fortune['energy_level']

        # 4. 调用 AI 生成批注
        user_bazi = {
            "year": profile.bazi_year,
            "month": profile.bazi_month,
            "day": profile.bazi_day,
            "hour": profile.bazi_hour
        }

        try:
            commentary = await ai_service.generate_almanac_commentary(
                bazi=user_bazi,
                ganzhi_day=bazi_today['day'],
                profession=profile.occupation,
                focus_areas=None  # notes 是文本备注，不再是列表
            )
        except Exception:
            commentary = "今日运势平稳，宜静思己过，勿急躁冒进。"

        # 5. 保存到数据库
        almanac = AlmanacHistory(
            user_id=user_id,
            date=target_date,
            lunar_date=lunar_info['full'], # e.g. 甲辰年正月初一
            ganzhi_year=bazi_today['year'],
            ganzhi_month=bazi_today['month'],
            ganzhi_day=bazi_today['day'],
            favorable=favorable,
            unfavorable=unfavorable,
            lucky_direction=lucky_direction,
            lucky_item=lucky_item,
            energy_level=energy_level,
            commentary=commentary,
            generated_at=datetime.now()
        )

        db.add(almanac)
        await db.commit()
        await db.refresh(almanac)

        return almanac
