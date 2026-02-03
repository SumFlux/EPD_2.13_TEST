from fastapi import APIRouter
from app.api.v1.endpoints import auth, user, almanac

api_router = APIRouter()

api_router.include_router(auth.router, prefix="/auth", tags=["认证"])
api_router.include_router(user.router, prefix="/user", tags=["用户档案"])
api_router.include_router(almanac.router, prefix="/almanac", tags=["智能黄历"])
