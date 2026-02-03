"""
通用模型基类
"""
from datetime import datetime
from typing import Any
from sqlalchemy import Column, Integer, DateTime
from app.core.database import Base


class BaseModel(Base):
    """所有模型的抽象基类"""
    __abstract__ = True

    id = Column(Integer, primary_key=True, index=True, autoincrement=True)
    created_at = Column(DateTime, default=datetime.now, nullable=False)
    updated_at = Column(DateTime, default=datetime.now, onupdate=datetime.now, nullable=False)

    def to_dict(self) -> dict[str, Any]:
        """转换为字典"""
        return {
            c.name: getattr(self, c.name)
            for c in self.__table__.columns
        }

    def __repr__(self) -> str:
        return f"<{self.__class__.__name__} id={self.id}>"
