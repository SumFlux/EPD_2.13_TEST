from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.ext.asyncio import AsyncSession
from typing import List
from app.api import deps
from app.services.divination_service import DivinationService
from app.schemas.divination import DivinationRequest, DivinationResponse
from app.models.user import User
import random

router = APIRouter()

# 常用字库 (简化版，实际应从 data/word_bank.json 读取)
COMMON_WORDS = ["天", "地", "人", "和", "道", "法", "术", "器", "名", "利", "情", "缘", "心", "性", "命", "运"]

@router.get("/words", response_model=List[str])
async def get_random_words(count: int = 8):
    """
    获取随机备选字
    """
    return random.sample(COMMON_WORDS, k=min(count, len(COMMON_WORDS)))

@router.post("/interpret", response_model=DivinationResponse)
async def interpret_divination(
    request: DivinationRequest,
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    解字测算
    """
    try:
        return await DivinationService.interpret(
            db,
            current_user.id,
            request.words,
            request.mode,
            request.intent
        )
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))
