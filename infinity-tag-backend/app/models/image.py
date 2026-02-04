from sqlalchemy import Column, String, Integer, ForeignKey
from sqlalchemy.orm import relationship
from app.models.base import BaseModel

class CustomImage(BaseModel):
    """
    用户自定义图片模型
    """
    __tablename__ = "custom_images"

    user_id = Column(Integer, ForeignKey("users.id", ondelete="CASCADE"), nullable=False, index=True)
    file_path = Column(String(255), nullable=False, comment='图片存储路径 (相对路径)')
    display_order = Column(Integer, default=0, comment='显示顺序')
    view_count = Column(Integer, default=0, comment='展示次数')

    # 关联
    user = relationship("User", back_populates="images")
