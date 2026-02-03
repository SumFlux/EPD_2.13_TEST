from pydantic import BaseModel, Field
from typing import Optional, List
from datetime import date as DateType, datetime

class AlmanacGenerateRequest(BaseModel):
    date: Optional[DateType] = Field(None, description="请求日期，默认今日")

class AlmanacResponse(BaseModel):
    date: DateType
    lunar_date: str

    ganzhi_year: str
    ganzhi_month: str
    ganzhi_day: str

    favorable: List[str]
    unfavorable: List[str]

    lucky_direction: Optional[str]
    lucky_item: Optional[str]
    energy_level: Optional[int]

    commentary: Optional[str]

    generated_at: datetime

    class Config:
        from_attributes = True

class AlmanacHistoryResponse(BaseModel):
    data: List[AlmanacResponse]
    total: int
