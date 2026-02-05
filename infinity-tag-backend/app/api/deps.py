from typing import AsyncGenerator
from fastapi import Depends, HTTPException, status
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials, OAuth2PasswordBearer
from jose import jwt, JWTError
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from pydantic import ValidationError

from app.config import settings
from app.core.database import AsyncSessionLocal
from app.models.user import User
from app.core.security import SecurityService
from fastapi import Request, Header
import hmac
import hashlib
import time

# 修改为 HTTPBearer，方便直接粘贴 Token
security_scheme = HTTPBearer()

# OAuth2 scheme for admin endpoints
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="/api/v1/admin/login", auto_error=True)

async def get_db() -> AsyncGenerator[AsyncSession, None]:
    async with AsyncSessionLocal() as session:
        try:
            yield session
        finally:
            await session.close()

async def get_current_user(
    db: AsyncSession = Depends(get_db),
    token: HTTPAuthorizationCredentials = Depends(security_scheme)
) -> User:
    try:
        # HTTPBearer 返回的是对象，credentials 属性才是 token 字符串
        payload = jwt.decode(
            token.credentials, settings.JWT_SECRET_KEY, algorithms=[settings.JWT_ALGORITHM]
        )
        token_data = payload.get("sub")
        if token_data is None:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Could not validate credentials",
            )
    except (JWTError, ValidationError):
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="Could not validate credentials",
        )

    # Check if token_data is user_id (int) or device_id (str)
    # Based on auth_service.py, we encoded user.id (int) into the subject
    try:
        user_id = int(token_data)
    except (ValueError, TypeError):
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="Invalid token subject",
        )

    stmt = select(User).where(User.id == user_id)
    result = await db.execute(stmt)
    user = result.scalars().first()

    if not user:
        raise HTTPException(status_code=404, detail="User not found")

    return user


async def get_temp_token_user_id(
    token: str = Depends(oauth2_scheme)
) -> int:
    """
    解析临时 token，返回用户 ID
    仅用于设置密码接口
    """
    try:
        payload = jwt.decode(
            token,
            settings.JWT_SECRET_KEY,
            algorithms=[settings.JWT_ALGORITHM]
        )
        subject = payload.get("sub")
        if not subject or not subject.startswith("temp:"):
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="无效的临时token"
            )
        return int(subject.split(":")[1])
    except JWTError:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="临时token已过期或无效"
        )
    except (ValueError, IndexError):
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="无效的token格式"
        )


def verify_admin_token(token: str = Depends(oauth2_scheme)) -> str:
    """验证管理员token"""
    try:
        payload = jwt.decode(
            token,
            settings.JWT_SECRET_KEY,
            algorithms=[settings.JWT_ALGORITHM]
        )
        subject = payload.get("sub")
        if not subject or not subject.startswith("admin:"):
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="需要管理员权限"
            )
        return subject.split(":")[1]
    except JWTError:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="token已过期或无效"
        )


async def verify_signed_request(
    request: Request,
    x_signature: str = Header(..., alias="X-Signature"),
    x_timestamp: int = Header(..., alias="X-Timestamp"),
    current_user: User = Depends(get_current_user)
) -> None:
    """
    验证请求签名 (HMAC-SHA256)
    Payload = Path + Timestamp + Body
    Secret = Device Init Password Hash (作为设备密钥)
    """
    # 1. 获取 Body
    body_bytes = await request.body()
    body_str = body_bytes.decode()

    # 2. 构造 Payload (与前端/设备端约定: Path + Timestamp + Body)
    # 实际构造: Path + Timestamp + Body
    payload = f"{request.url.path}{x_timestamp}{body_str}"

    # 3. 获取 Secret (假设使用设备初始密码哈希作为密钥，这是设备独有的)
    # 如果用户未绑定设备，无法验证
    if not current_user.device:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="用户未绑定设备，无法验证签名"
        )
    
    secret = current_user.device.init_password_hash

    # 4. 验证
    # 检查时间戳 (例如 5 分钟内)
    if abs(time.time() - x_timestamp) > 300:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="请求时间戳无效或已过期"
        )

    expected_signature = hmac.new(
        secret.encode(),
        payload.encode(),
        hashlib.sha256
    ).hexdigest()

    if not hmac.compare_digest(x_signature, expected_signature):
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="签名验证失败"
        )
