from sqlalchemy import Column, String, Text
from app.models.base import BaseModel

class SystemConfig(BaseModel):
    """
    系统配置模型
    """
    __tablename__ = "system_config"

    config_key = Column(String(100), unique=True, nullable=False)
    config_value = Column(Text)
    description = Column(String(255))
