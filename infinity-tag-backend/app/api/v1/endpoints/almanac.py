from datetime import date
from fastapi import APIRouter, Depends
from sqlalchemy.ext.asyncio import AsyncSession
from app.api import deps
from app.models.user import User
from app.services.almanac_service import AlmanacService
from app.schemas.almanac import AlmanacRequest, AlmanacResponse
from app.schemas.common import ResponseBase

router = APIRouter()


@router.post("/generate", response_model=ResponseBase[AlmanacResponse])
async def generate_almanac(
    request: AlmanacRequest,
    current_user: User = Depends(deps.get_current_user),
    db: AsyncSession = Depends(deps.get_db)
):
    """
    生成或获取黄历
    如果不传 target_date，默认为今天
    """
    target_date = request.target_date or date.today()

    almanac = await AlmanacService.get_daily_almanac(
        db,
        current_user.id,
        target_date
    )

    return ResponseBase(data=almanac)
