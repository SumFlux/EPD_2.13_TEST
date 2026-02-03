from fastapi import APIRouter, Depends
from sqlalchemy.ext.asyncio import AsyncSession
from app.api import deps
from app.models.user import User
from app.services.user_service import UserService
from app.schemas.user import UserProfileCreate, UserProfileResponse
from app.schemas.common import ResponseBase

router = APIRouter()


@router.get("/profile", response_model=ResponseBase[UserProfileResponse])
async def get_profile(
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    获取当前用户档案
    """
    profile = await UserService.get_profile(db, current_user.id)
    if not profile:
        # 注意: 这里的 message 会被前端用来判断显示"去设置"按钮
        return ResponseBase(success=False, message="档案未设置", error_code=1002)

    return ResponseBase(data=profile)


@router.post("/profile", response_model=ResponseBase[UserProfileResponse])
async def create_or_update_profile(
    data: UserProfileCreate,
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    设置或更新档案
    需要提供完整的生辰八字信息
    """
    profile = await UserService.create_or_update_profile(db, current_user.id, data)
    return ResponseBase(data=profile)
