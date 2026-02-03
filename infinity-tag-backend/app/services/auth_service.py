from datetime import datetime, timezone
from fastapi import HTTPException, status
from sqlalchemy.ext.asyncio import AsyncSession
from app.repositories.user_repo import UserRepository
from app.core.security import SecurityService, create_access_token, verify_password, get_password_hash
from app.schemas.auth import DeviceActivateRequest, DeviceLoginRequest, ActivationResponse, TokenResponse
from app.utils.device_code_gen import generate_device_code
import secrets

class AuthService:
    def __init__(self, db: AsyncSession):
        self.user_repo = UserRepository(db)

    async def activate_device(self, request: DeviceActivateRequest) -> ActivationResponse:
        # 1. Check if device exists
        existing_user = await self.user_repo.get_by_device_id(request.device_id)
        if existing_user:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Device already activated"
            )

        # 2. Create new user
        device_secret = secrets.token_hex(32)
        user_data = {
            "device_id": request.device_id,
            "password_hash": get_password_hash(request.password),
            "device_secret": device_secret,
            "activated_at": datetime.now(timezone.utc)
        }

        user = await self.user_repo.create_user(user_data)

        # 3. Generate token
        token = create_access_token(subject=user.id)

        # 4. Generate QR URL (Placeholder)
        qr_url = f"https://infinitytag.app/setup?did={user.device_id}"

        return ActivationResponse(
            success=True,
            token=token,
            device_secret=device_secret,
            qr_url=qr_url
        )

    async def login(self, request: DeviceLoginRequest) -> TokenResponse:
        user = await self.user_repo.get_by_device_id(request.device_id)
        if not user or not verify_password(request.password, user.password_hash):
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Incorrect device ID or password"
            )

        await self.user_repo.update_login_time(user)
        access_token = create_access_token(subject=user.id)

        return TokenResponse(access_token=access_token)
