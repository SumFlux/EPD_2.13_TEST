"""
固件模型 - OTA 升级用
"""
from sqlalchemy import Column, String, Integer, Boolean, Text
from app.models.base import BaseModel


class Firmware(BaseModel):
    """
    固件版本管理
    """
    __tablename__ = "firmwares"

    version_code = Column(
        Integer,
        unique=True,
        index=True,
        nullable=False,
        comment='整数版本号 (A.B.C.D -> A*1000000 + B*10000 + C*100 + D)'
    )
    version_str = Column(
        String(20),
        unique=True,
        nullable=False,
        comment='显示版本号 (如 1.0.0.1)'
    )
    file_path = Column(
        String(255),
        nullable=False,
        comment='固件文件存储路径'
    )
    checksum = Column(
        String(64),
        nullable=False,
        comment='SHA-256 校验和'
    )
    is_active = Column(
        Boolean,
        default=True,
        comment='是否可用'
    )
    description = Column(
        Text,
        nullable=True,
        comment='版本更新说明'
    )
