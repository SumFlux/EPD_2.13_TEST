from pydantic import BaseModel, Field
from typing import Optional, List
from datetime import datetime

class ProfileBase(BaseModel):
    birth_datetime: datetime
    birth_place: Optional[str] = None
    profession: Optional[str] = None
    focus_areas: Optional[List[str]] = Field(default_factory=list)

class ProfileCreate(ProfileBase):
    pass

class ProfileUpdate(BaseModel):
    birth_datetime: Optional[datetime] = None
    birth_place: Optional[str] = None
    profession: Optional[str] = None
    focus_areas: Optional[List[str]] = None

class ProfileResponse(ProfileBase):
    id: int
    user_id: int
    bazi_year: str
    bazi_month: str
    bazi_day: str
    bazi_hour: str

    class Config:
        from_attributes = True
