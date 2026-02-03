"""
黄历历史模型
"""
from sqlalchemy import Column, String, Integer, ForeignKey, Text, Date
from sqlalchemy.orm import relationship
from app.models.base import BaseModel


class AlmanacHistory(BaseModel):
    """
    黄历历史记录
    记录用户每天生成的个性化黄历
    """
    __tablename__ = "almanac_history"

    user_id = Column(Integer, ForeignKey("users.id"), nullable=False, index=True)
    date = Column(Date, nullable=False, index=True, comment="黄历日期")

    # 宜忌
    auspicious = Column(String(255), nullable=True, comment="宜")
    inauspicious = Column(String(255), nullable=True, comment="忌")

    # AI 生成内容
    daily_fortune = Column(Text, nullable=True, comment="今日运势")
    lucky_color = Column(String(32), nullable=True, comment="幸运色")
    lucky_direction = Column(String(32), nullable=True, comment="财神方位")
    lucky_time = Column(String(64), nullable=True, comment="吉时")

    # 结构化评分 (0-100)
    wealth_score = Column(Integer, default=60, comment="财运分")
    health_score = Column(Integer, default=60, comment="健康分")
    love_score = Column(Integer, default=60, comment="桃花分")
    career_score = Column(Integer, default=60, comment="事业分")

    # 关联
    user = relationship("User", back_populates="almanacs")
