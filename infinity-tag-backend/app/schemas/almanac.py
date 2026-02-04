from pydantic import BaseModel, Field, field_validator
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

    favorable: str  # 宜，用顿号分隔
    unfavorable: str  # 忌，用顿号分隔

    lucky_direction: Optional[str]
    lucky_item: Optional[str]
    energy_level: Optional[int]

    commentary: Optional[str]

    generated_at: datetime

    @field_validator('favorable', 'unfavorable', mode='before')
    @classmethod
    def convert_list_to_string(cls, v):
        """将列表转换为顿号分隔的字符串"""
        if isinstance(v, list):
            return '、'.join(v)
        return v

    class Config:
        from_attributes = True

class AlmanacHistoryResponse(BaseModel):
    data: List[AlmanacResponse]
    total: int
