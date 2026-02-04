from typing import Optional
from pydantic import BaseModel, Field, field_validator
from datetime import datetime

class ImageBase(BaseModel):
    display_order: Optional[int] = 0

class ImageCreate(ImageBase):
    pass

class ImageUpdate(ImageBase):
    pass

class ImageOut(ImageBase):
    id: int
    url: str
    view_count: int
    created_at: datetime

    class Config:
        from_attributes = True

class ImageReorder(BaseModel):
    id: int
    new_order: int

class ImageProcessOptions(BaseModel):
    """图片处理参数"""
    # 裁剪区域 (绝对像素)
    crop_x: Optional[int] = Field(None, ge=0)
    crop_y: Optional[int] = Field(None, ge=0)
    crop_w: Optional[int] = Field(None, gt=0)
    crop_h: Optional[int] = Field(None, gt=0)

    # 旋转 (仅支持特定角度)
    rotate: int = Field(0, description="旋转角度: 0, 90, 180, 270")

    # 颜色处理
    invert: bool = False
    dither: bool = True     # True=抖动, False=阈值
    threshold: int = Field(128, ge=0, le=255)

    @field_validator('rotate')
    def validate_rotate(cls, v):
        if v not in [0, 90, 180, 270]:
            raise ValueError('Rotate must be 0, 90, 180, or 270')
        return v
