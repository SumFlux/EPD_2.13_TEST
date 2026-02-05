"""
导出所有模型，方便 Alembic 自动发现
"""
from app.models.base import Base, BaseModel
from app.models.user import User, UserProfile
from app.models.device import Device, DeviceStatus
from app.models.almanac import AlmanacHistory
from app.models.divination import DivinationRecord
from app.models.image import CustomImage
from app.models.system import SystemConfig
from app.models.firmware import Firmware

# 方便导入
__all__ = [
    "Base",
    "BaseModel",
    "User",
    "UserProfile",
    "Device",
    "DeviceStatus",
    "AlmanacHistory",
    "DivinationRecord",
    "CustomImage",
    "SystemConfig",
    "Firmware"
]
