from pydantic import BaseModel, Field
from typing import List, Optional
from datetime import datetime

class DivinationRequest(BaseModel):
    mode: str = Field(..., pattern="^(本命|客座)$", description="测算模式")
    intent: str = Field(..., max_length=10, description="求测意图，如'事业'")
    words: List[str] = Field(..., min_items=2, max_items=2, description="选中的两个字")

class DivinationResponse(BaseModel):
    id: int
    mode: str
    intent: str
    selected_words: List[str]
    result_idiom: Optional[str]
    result_interpretation: Optional[str]
    result_advice: Optional[str]
    created_at: datetime

    class Config:
        from_attributes = True
