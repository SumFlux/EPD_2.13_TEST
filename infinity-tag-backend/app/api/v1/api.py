from fastapi import APIRouter
from app.api.v1.endpoints import auth, profile, almanac, divination, renderer

api_router = APIRouter()

api_router.include_router(auth.router, prefix="/auth", tags=["Auth"])
api_router.include_router(profile.router, prefix="/profile", tags=["Profile"])
api_router.include_router(almanac.router, prefix="/almanac", tags=["Almanac"])
api_router.include_router(divination.router, prefix="/divination", tags=["Divination"])
api_router.include_router(renderer.router, prefix="/renderer", tags=["Renderer"])
