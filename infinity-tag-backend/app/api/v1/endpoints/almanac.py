from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.ext.asyncio import AsyncSession
from typing import List
from datetime import date
from app.api import deps
from app.services.almanac_service import AlmanacService
from app.services.profile_service import ProfileService
from app.schemas.almanac import AlmanacResponse, AlmanacGenerateRequest, AlmanacHistoryResponse
from app.models.user import User

router = APIRouter()

@router.post("/generate", response_model=AlmanacResponse)
async def generate_almanac(
    request: AlmanacGenerateRequest,
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    生成黄历 (默认今日)
    """
    target_date = request.date if request.date else date.today()

    # 必须先有档案
    profile = await ProfileService.get_profile(db, current_user.id)
    if not profile:
        raise HTTPException(status_code=400, detail="请先完善用户档案 (Profile)")

    return await AlmanacService.generate_almanac(db, current_user.id, profile, target_date)

@router.get("/history", response_model=AlmanacHistoryResponse)
async def get_history(
    limit: int = 30,
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    获取历史黄历
    """
    history = await AlmanacService.get_history(db, current_user.id, limit)
    return {"data": history, "total": len(history)}
