from typing import Optional, Literal
from pydantic import BaseModel, Field, field_validator
from datetime import datetime
from enum import Enum

class DitherAlgorithm(str, Enum):
    """抖动算法类型"""
    FLOYD_STEINBERG = "floyd_steinberg"  # 经典抖动
    ATKINSON = "atkinson"                # E-Ink推荐
    BAYER = "bayer"                      # 有序抖动
    THRESHOLD = "threshold"              # 简单阈值

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
    
    # 抖动算法选择 (新增)
    dither_algorithm: DitherAlgorithm = Field(
        DitherAlgorithm.ATKINSON, 
        description="抖动算法: floyd_steinberg, atkinson, bayer, threshold"
    )
    
    # 阈值 (用于 threshold 算法)
    threshold: int = Field(128, ge=0, le=255)
    
    # 对比度增强 (1.0=不变, 1.5=增强50%) 
    contrast: float = Field(1.3, ge=0.5, le=3.0, description="对比度增强系数")
    
    # 锐化增强 (1.0=不变, 2.0=增强100%)
    sharpness: float = Field(1.5, ge=0.0, le=5.0, description="锐化增强系数")
    
    # Gamma 校正 (1.0=不变, >1=变暗, <1=变亮)
    gamma: float = Field(1.2, ge=0.3, le=3.0, description="Gamma校正系数")

    # 兼容旧接口 (deprecated)
    dither: bool = Field(True, deprecated=True, description="已废弃，请使用 dither_algorithm")

    @field_validator('rotate')
    def validate_rotate(cls, v):
        if v not in [0, 90, 180, 270]:
            raise ValueError('Rotate must be 0, 90, 180, or 270')
        return v
