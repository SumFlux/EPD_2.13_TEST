"""
黄历服务
核心业务逻辑：协调 AI 生成与数据库缓存
"""
from datetime import date
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from fastapi import HTTPException, status
from loguru import logger

from app.models.almanac import AlmanacHistory
from app.models.user import UserProfile
from app.schemas.almanac import AlmanacResponse
from app.services.ai_service import ai_service
from app.services.user_service import UserService


class AlmanacService:
    """黄历业务逻辑"""

    @staticmethod
    async def get_daily_almanac(
        db: AsyncSession,
        user_id: int,
        target_date: date
    ) -> AlmanacResponse:
        """
        获取指定日期的黄历
        策略: 优先查库 -> 库中无则调用AI生成 -> 存库并返回
        """
        # 1. 查库 (Cache Check)
        stmt = select(AlmanacHistory).where(
            AlmanacHistory.user_id == user_id,
            AlmanacHistory.date == target_date
        )
        result = await db.execute(stmt)
        cached_almanac = result.scalar_one_or_none()

        if cached_almanac:
            logger.info(f"Cache Hit: User {user_id} Date {target_date}")
            # 补全 Pydantic 模型所需的 date 字段（数据库对象转dict需要注意）
            # Pydantic from_attributes=True 可以处理 ORM 对象
            return AlmanacResponse.model_validate(cached_almanac)

        logger.info(f"Cache Miss: Generating for User {user_id} Date {target_date}")

        # 2. 准备生成: 获取用户档案
        profile = await UserService.get_profile(db, user_id)
        if not profile:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="请先完善个人档案(设置生辰八字)才能生成黄历"
            )

        # 3. 调用 AI 生成
        # 注意: ai_service 返回的是字典数据 (core content)
        try:
            ai_data = await ai_service.generate_almanac(profile, target_date)
        except Exception as e:
            logger.error(f"AI Generation Failed: {e}")
            raise HTTPException(
                status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
                detail="AI 服务暂时不可用，请稍后重试"
            )

        # 4. 存入数据库
        # 将 AI 返回的字典数据解包，填入模型
        # 注意: AI 返回的字段必须与 AlmanacHistory 模型字段匹配
        # 我们需要处理一下 list 类型的 auspicious/inauspicious 转字符串存储

        # 将 list 转为 comma-separated string 存储到数据库
        auspicious_str = ",".join(ai_data.get("auspicious", []))
        inauspicious_str = ",".join(ai_data.get("inauspicious", []))

        new_almanac = AlmanacHistory(
            user_id=user_id,
            date=target_date,
            auspicious=auspicious_str,
            inauspicious=inauspicious_str,
            daily_fortune=ai_data.get("daily_fortune"),
            lucky_color=ai_data.get("lucky_color"),
            lucky_direction=ai_data.get("lucky_direction"),
            lucky_time=ai_data.get("lucky_time"),
            wealth_score=ai_data.get("wealth_score"),
            health_score=ai_data.get("health_score"),
            love_score=ai_data.get("love_score"),
            career_score=ai_data.get("career_score")
        )

        db.add(new_almanac)
        await db.commit()
        await db.refresh(new_almanac)

        # 5. 返回结果
        # 返回时，ORM 对象会自动将 comma string 转回 list 吗？不会。
        # 我们需要在 Pydantic model_validate 之前手动处理，或者让 Pydantic validator 处理。
        # 更简单的做法：构造一个临时的 dict 来返回，确保 auspicious 是 list

        response_data = new_almanac.to_dict()
        # 手动转换一下 list
        response_data["auspicious"] = ai_data.get("auspicious", [])
        response_data["inauspicious"] = ai_data.get("inauspicious", [])

        # 补充农历日期字符串 (可选，这里简单从 LunarUtils 获取)
        # 实际上 ai_service 并没有返回 lunar_date_str，我们可以补上
        from app.utils.lunar import LunarUtils
        lunar_info = LunarUtils.solar_to_lunar(target_date)
        response_data["lunar_date_str"] = lunar_info["full"]

        return AlmanacResponse(**response_data)
