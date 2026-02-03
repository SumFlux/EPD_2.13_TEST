from pydantic import BaseModel, Field, validator
from typing import Optional, List
from datetime import datetime

class ProfileCreate(BaseModel):
    birth_date: datetime = Field(..., description="出生日期时间 (公历)")
    birth_place: Optional[str] = Field(None, max_length=100)
    profession: Optional[str] = Field(None, max_length=50)
    focus_areas: List[str] = Field(default_factory=list, description="关注领域，如['事业', '财运']")

class ProfileUpdate(BaseModel):
    birth_place: Optional[str] = None
    profession: Optional[str] = None
    focus_areas: Optional[List[str]] = None

class ProfileResponse(BaseModel):
    user_id: int
    birth_datetime: datetime
    birth_place: Optional[str]

    # 八字
    bazi_year: str
    bazi_month: str
    bazi_day: str
    bazi_hour: str

    profession: Optional[str]
    focus_areas: Optional[List[str]]

    class Config:
        from_attributes = True
