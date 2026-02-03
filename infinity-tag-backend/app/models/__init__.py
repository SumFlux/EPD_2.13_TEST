"""
导出所有模型，方便 Alembic 自动发现
"""
from app.models.base import Base, BaseModel
from app.models.user import User, UserProfile
from app.models.almanac import AlmanacHistory
from app.models.divination import DivinationRecord
from app.models.system import SystemConfig

# 方便导入
__all__ = [
    "Base",
    "BaseModel",
    "User",
    "UserProfile",
    "AlmanacHistory",
    "DivinationRecord",
    "SystemConfig"
]
