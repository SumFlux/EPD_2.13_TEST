from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.ext.asyncio import AsyncSession
from app.api import deps
from app.services.profile_service import ProfileService
from app.schemas.profile import ProfileCreate, ProfileResponse, ProfileUpdate
from app.models.user import User

router = APIRouter()

@router.get("/", response_model=ProfileResponse)
async def get_profile(
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    获取当前用户的档案信息
    """
    profile = await ProfileService.get_profile(db, current_user.id)
    if not profile:
        raise HTTPException(status_code=404, detail="档案不存在")
    return profile

@router.post("/", response_model=ProfileResponse)
async def create_or_update_profile(
    profile_in: ProfileCreate,
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    创建或完全更新用户档案
    """
    return await ProfileService.create_or_update_profile(db, current_user.id, profile_in)

@router.patch("/", response_model=ProfileResponse)
async def update_profile_partial(
    profile_in: ProfileUpdate,
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    部分更新用户档案
    """
    profile = await ProfileService.update_profile_partial(db, current_user.id, profile_in)
    if not profile:
        raise HTTPException(status_code=404, detail="档案不存在")
    return profile
