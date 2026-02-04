from sqlalchemy import Column, String, DateTime, Integer, ForeignKey, JSON, Boolean
from sqlalchemy.orm import relationship
from app.models.base import BaseModel

class User(BaseModel):
    """
    用户模型 (设备影子账户)
    """
    __tablename__ = "users"

    device_id = Column(String(10), unique=True, index=True, nullable=False, comment='6位设备短码')
    password_hash = Column(String(255), nullable=True, comment='用户自定义密码哈希')
    device_secret = Column(String(64), nullable=False, comment='HMAC密钥')
    password_set = Column(Boolean, default=False, nullable=False, comment='用户是否已设置自定义密码')
    activated_at = Column(DateTime, nullable=True)
    last_login_at = Column(DateTime, nullable=True)

    # 关联
    profile = relationship("UserProfile", back_populates="user", uselist=False, cascade="all, delete-orphan")
    almanacs = relationship("AlmanacHistory", back_populates="user", cascade="all, delete-orphan")
    divination_records = relationship("DivinationRecord", back_populates="user", cascade="all, delete-orphan")
    images = relationship("CustomImage", back_populates="user", cascade="all, delete-orphan")
    device = relationship("Device", back_populates="user", uselist=False)

class UserProfile(BaseModel):
    """
    用户档案模型 (八字信息)
    """
    __tablename__ = "user_profiles"

    user_id = Column(Integer, ForeignKey("users.id", ondelete="CASCADE"), unique=True, nullable=False, index=True)

    birth_datetime = Column(DateTime, nullable=False)
    birth_place = Column(String(100), nullable=True)

    # 八字四柱
    bazi_year = Column(String(10), nullable=False)
    bazi_month = Column(String(10), nullable=False)
    bazi_day = Column(String(10), nullable=False)
    bazi_hour = Column(String(10), nullable=False)

    profession = Column(String(50), nullable=True)
    focus_areas = Column(JSON, nullable=True, comment='["事业", "财运"]')

    # 关联
    user = relationship("User", back_populates="profile")
