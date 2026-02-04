from pydantic import BaseModel, Field
from typing import Optional, List
from datetime import datetime

class ProfileCreate(BaseModel):
    """前端提交的档案创建请求"""
    nickname: str = Field(..., min_length=1, max_length=50)
    gender: int = Field(..., ge=0, le=1)  # 0: 女, 1: 男
    birth_year: int = Field(..., ge=1900, le=2100)
    birth_month: int = Field(..., ge=1, le=12)
    birth_day: int = Field(..., ge=1, le=31)
    birth_hour: int = Field(default=-1, ge=-1, le=23)  # -1 表示未知
    is_lunar: bool = False  # 是否为农历
    occupation: Optional[str] = None
    notes: Optional[str] = None

class ProfileUpdate(BaseModel):
    """部分更新档案"""
    nickname: Optional[str] = Field(None, min_length=1, max_length=50)
    gender: Optional[int] = Field(None, ge=0, le=1)
    birth_year: Optional[int] = Field(None, ge=1900, le=2100)
    birth_month: Optional[int] = Field(None, ge=1, le=12)
    birth_day: Optional[int] = Field(None, ge=1, le=31)
    birth_hour: Optional[int] = Field(None, ge=-1, le=23)
    is_lunar: Optional[bool] = None
    occupation: Optional[str] = None
    notes: Optional[str] = None

class ProfileResponse(BaseModel):
    """档案响应"""
    id: int
    user_id: int
    nickname: str
    gender: int
    birth_year: int
    birth_month: int
    birth_day: int
    birth_hour: int
    is_lunar: bool
    occupation: Optional[str] = None
    notes: Optional[str] = None
    # 八字信息 (后端计算)
    bazi_year: str
    bazi_month: str
    bazi_day: str
    bazi_hour: str
    created_at: datetime
    updated_at: Optional[datetime] = None

    class Config:
        from_attributes = True
