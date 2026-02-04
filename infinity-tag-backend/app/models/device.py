"""
设备模型 - 管理员录入的设备信息
"""
from sqlalchemy import Column, String, DateTime, Integer, ForeignKey, Text
from sqlalchemy.orm import relationship
from app.models.base import BaseModel
import enum


class DeviceStatus(str, enum.Enum):
    """设备状态枚举"""
    PENDING = "pending"      # 待激活（初始密码有效）
    ACTIVATED = "activated"  # 已激活（初始密码失效，用户密码有效）
    DISABLED = "disabled"    # 已禁用


class Device(BaseModel):
    """
    设备模型 - 由管理员录入
    """
    __tablename__ = "devices"

    device_code = Column(
        String(6),
        unique=True,
        index=True,
        nullable=False,
        comment='6位设备码（由UUID通过算法A生成）'
    )
    init_password_hash = Column(
        String(255),
        nullable=False,
        comment='初始密码哈希（由UUID通过算法B生成）'
    )
    uuid = Column(
        String(64),
        unique=True,
        nullable=False,
        comment='ESP32芯片UUID'
    )
    status = Column(
        String(20),
        default=DeviceStatus.PENDING.value,
        nullable=False,
        comment='pending/activated/disabled'
    )
    user_id = Column(
        Integer,
        ForeignKey("users.id", ondelete="SET NULL"),
        nullable=True,
        comment='激活后关联的用户ID'
    )
    batch_name = Column(
        String(100),
        nullable=True,
        comment='批次名称'
    )
    activated_at = Column(
        DateTime,
        nullable=True,
        comment='激活时间'
    )
    notes = Column(
        Text,
        nullable=True,
        comment='备注'
    )

    # 关联
    user = relationship("User", back_populates="device", uselist=False)
