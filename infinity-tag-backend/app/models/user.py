"""
用户与档案模型
"""
from sqlalchemy import Column, String, Boolean, Integer, ForeignKey, SmallInteger, Text
from sqlalchemy.orm import relationship
from app.models.base import BaseModel


class User(BaseModel):
    """
    用户模型 (对应设备)
    """
    __tablename__ = "users"

    device_code = Column(String(6), unique=True, index=True, nullable=False, comment="6位设备短码")
    device_uuid = Column(String(64), unique=True, index=True, nullable=True, comment="设备硬件ID hash")
    is_active = Column(Boolean, default=True, comment="是否激活")
    last_login_at = Column(String(32), nullable=True, comment="最后登录时间")

    # 关联
    profile = relationship("UserProfile", back_populates="user", uselist=False, cascade="all, delete-orphan")
    almanacs = relationship("AlmanacHistory", back_populates="user", cascade="all, delete-orphan")


class UserProfile(BaseModel):
    """
    用户档案模型 (生辰八字)
    """
    __tablename__ = "user_profiles"

    user_id = Column(Integer, ForeignKey("users.id"), unique=True, nullable=False, index=True)

    nickname = Column(String(32), default="有缘人", comment="昵称")
    gender = Column(SmallInteger, default=1, comment="性别: 1男 0女")

    # 生辰信息
    birth_year = Column(Integer, nullable=False, comment="出生年")
    birth_month = Column(Integer, nullable=False, comment="出生月")
    birth_day = Column(Integer, nullable=False, comment="出生日")
    birth_hour = Column(Integer, default=-1, comment="出生时辰(0-23, -1未知)")

    is_lunar = Column(Boolean, default=False, comment="是否为农历生日")

    # 扩展信息
    occupation = Column(String(32), nullable=True, comment="职业/身份")
    notes = Column(Text, nullable=True, comment="备注")

    # 关联
    user = relationship("User", back_populates="profile")
