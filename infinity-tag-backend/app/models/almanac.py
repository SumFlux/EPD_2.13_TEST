from sqlalchemy import Column, String, Integer, ForeignKey, Text, Date, JSON, UniqueConstraint, DateTime
from sqlalchemy.orm import relationship
from app.models.base import BaseModel
from datetime import datetime

class AlmanacHistory(BaseModel):
    """
    黄历历史模型
    """
    __tablename__ = "almanac_history"

    user_id = Column(Integer, ForeignKey("users.id", ondelete="CASCADE"), nullable=False, index=True)
    date = Column(Date, nullable=False, index=True)

    lunar_date = Column(String(20))
    ganzhi_year = Column(String(10))
    ganzhi_month = Column(String(10))
    ganzhi_day = Column(String(10))

    favorable = Column(JSON, comment='宜事项')
    unfavorable = Column(JSON, comment='忌事项')

    lucky_direction = Column(String(20))
    lucky_item = Column(String(50))
    energy_level = Column(Integer)

    commentary = Column(Text, comment='AI批注')
    generated_at = Column(DateTime, default=datetime.now)

    # 关联
    user = relationship("User", back_populates="almanacs")

    __table_args__ = (
        UniqueConstraint('user_id', 'date', name='unique_user_date'),
    )
