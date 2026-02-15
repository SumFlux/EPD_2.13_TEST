from sqlalchemy import Column, String, DateTime, Integer, ForeignKey, JSON, Boolean, Text
from sqlalchemy.orm import relationship
from app.models.base import BaseModel
import enum


class DeviceStatus(str, enum.Enum):
    """设备状态枚举"""
    PENDING = "pending"      # 待激活（初始密码有效）
    ACTIVATED = "activated"  # 已激活（用户密码有效）
    DISABLED = "disabled"    # 已禁用


class User(BaseModel):
    """
    用户模型 (设备即用户)
    """
    __tablename__ = "users"

    # 设备相关字段（来自原 Device 表）
    device_code = Column(String(10), unique=True, index=True, nullable=False, comment='设备码（兼容历史数据）')
    init_password_hash = Column(String(255), nullable=False, comment='初始密码哈希')
    uuid = Column(String(64), unique=True, nullable=False, comment='ESP32芯片UUID')
    status = Column(String(20), default=DeviceStatus.PENDING.value, nullable=False, comment='设备状态: pending/activated/disabled')

    # 用户相关字段
    password_hash = Column(String(255), nullable=True, comment='用户自定义密码哈希')
    device_secret = Column(String(64), nullable=False, comment='HMAC密钥')
    password_set = Column(Boolean, default=False, nullable=False, comment='用户是否已设置自定义密码')
    activated_at = Column(DateTime, nullable=True, comment='激活时间')
    last_login_at = Column(DateTime, nullable=True, comment='最后登录时间')

    # 关联
    profile = relationship("UserProfile", back_populates="user", uselist=False, cascade="all, delete-orphan")
    almanacs = relationship("AlmanacHistory", back_populates="user", cascade="all, delete-orphan")
    divination_records = relationship("DivinationRecord", back_populates="user", cascade="all, delete-orphan")
    images = relationship("CustomImage", back_populates="user", cascade="all, delete-orphan")

class UserProfile(BaseModel):
    """
    用户档案模型 (八字信息)
    """
    __tablename__ = "user_profiles"

    user_id = Column(Integer, ForeignKey("users.id", ondelete="CASCADE"), unique=True, nullable=False, index=True)

    # 基本信息
    nickname = Column(String(50), nullable=False, comment='昵称')
    gender = Column(Integer, nullable=False, default=1, comment='性别: 0女 1男')

    # 出生信息 (分开存储，支持农历)
    birth_year = Column(Integer, nullable=False)
    birth_month = Column(Integer, nullable=False)
    birth_day = Column(Integer, nullable=False)
    birth_hour = Column(Integer, nullable=False, default=-1, comment='时辰: -1未知, 0-23')
    is_lunar = Column(Boolean, nullable=False, default=False, comment='是否农历')

    # 八字四柱 (后端计算)
    bazi_year = Column(String(10), nullable=False)
    bazi_month = Column(String(10), nullable=False)
    bazi_day = Column(String(10), nullable=False)
    bazi_hour = Column(String(10), nullable=False)

    # 其他信息
    occupation = Column(String(50), nullable=True, comment='职业')
    notes = Column(Text, nullable=True, comment='备注/关注点')

    # 关联
    user = relationship("User", back_populates="profile")
