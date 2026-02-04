from fastapi import APIRouter, Depends, HTTPException, Query
from sqlalchemy.ext.asyncio import AsyncSession
from typing import List, Optional
from app.api import deps
from app.services.divination_service import DivinationService
from app.services.word_bank_service import WordBankService
from app.schemas.divination import DivinationRequest, DivinationResponse
from app.models.user import User

router = APIRouter()


@router.get("/words", response_model=List[str])
async def get_random_words(
    count: int = Query(default=8, ge=1, le=20, description="获取字数"),
    category: Optional[str] = Query(default=None, description="字库类别")
):
    """
    获取随机备选字

    - **count**: 获取的字数 (1-20)
    - **category**: 可选类别筛选 (天象/地理/人事/器物/德行/玄学/自然/动物/时令/情志/方位/数理)
    """
    service = WordBankService.get_instance()
    return service.get_random_words(count, category)


@router.get("/categories", response_model=List[str])
async def get_word_categories():
    """
    获取所有可用的字库类别
    """
    service = WordBankService.get_instance()
    return service.get_all_categories()

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
