"""
黄历 Schema
"""
from datetime import date
from typing import Optional, List
from pydantic import BaseModel, Field


class AlmanacRequest(BaseModel):
    """
    请求生成黄历
    通常不需要参数，默认生成当天的，或指定日期
    """
    target_date: Optional[date] = Field(None, description="指定日期 (默认今天)")


class AlmanacResponse(BaseModel):
    """
    单日黄历响应
    """
    id: int
    date: date

    # 基础宜忌 (列表格式，方便前端展示)
    auspicious: List[str] = Field(default_factory=list, description="宜 (数组)")
    inauspicious: List[str] = Field(default_factory=list, description="忌 (数组)")

    # AI 生成内容
    daily_fortune: str = Field(..., description="今日运势解读")
    lucky_color: str = Field(..., description="幸运色")
    lucky_direction: str = Field(..., description="财神方位")
    lucky_time: str = Field(..., description="吉时")

    # 结构化评分 (0-100)
    wealth_score: int = Field(..., ge=0, le=100, description="财运分")
    health_score: int = Field(..., ge=0, le=100, description="健康分")
    love_score: int = Field(..., ge=0, le=100, description="桃花分")
    career_score: int = Field(..., ge=0, le=100, description="事业分")

    # 农历信息 (可选，由前端计算或后端补充)
    lunar_date_str: Optional[str] = Field(None, description="农历日期字符串 (如: 甲辰年正月初一)")

    class Config:
        from_attributes = True


class AlmanacSummary(BaseModel):
    """
    黄历简略信息 (用于列表)
    """
    date: date
    daily_fortune: str
    overall_score: int = Field(..., description="综合运势分 (平均分)")

    class Config:
        from_attributes = True
