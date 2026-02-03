from sqlalchemy import Column, String, Integer, ForeignKey, Text, JSON
from sqlalchemy.orm import relationship
from app.models.base import BaseModel

class DivinationRecord(BaseModel):
    """
    解字记录模型
    """
    __tablename__ = "divination_records"

    user_id = Column(Integer, ForeignKey("users.id", ondelete="CASCADE"), nullable=False)

    mode = Column(String(10), nullable=False, comment='本命/客座')
    intent = Column(String(20))
    selected_words = Column(JSON, nullable=False)

    result_idiom = Column(String(50))
    result_interpretation = Column(Text)
    result_advice = Column(Text)

    # 关联
    user = relationship("User", back_populates="divination_records")
