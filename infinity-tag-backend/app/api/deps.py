"""
API 依赖注入
处理数据库会话获取与用户认证
"""
from typing import AsyncGenerator, Annotated
from fastapi import Depends, HTTPException, status
from fastapi.security import OAuth2PasswordBearer
from jose import jwt, JWTError
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select

from app.core.database import AsyncSessionLocal
from app.config import settings
from app.models.user import User

# 定义 OAuth2 认证方案 (Token URL 仅作为文档展示，实际使用 activate 接口)
oauth2_scheme = OAuth2PasswordBearer(tokenUrl=f"/api/v1/auth/activate")


async def get_db() -> AsyncGenerator[AsyncSession, None]:
    """
    获取数据库会话 (依赖注入)
    """
    async with AsyncSessionLocal() as session:
        try:
            yield session
            await session.commit()
        except Exception:
            await session.rollback()
            raise
        finally:
            await session.close()


async def get_current_user(
    token: Annotated[str, Depends(oauth2_scheme)],
    db: Annotated[AsyncSession, Depends(get_db)]
) -> User:
    """
    获取当前登录用户 (依赖注入)
    验证 JWT Token 并查询数据库
    """
    credentials_exception = HTTPException(
        status_code=status.HTTP_401_UNAUTHORIZED,
        detail="认证凭证无效或已过期",
        headers={"WWW-Authenticate": "Bearer"},
    )

    try:
        # 解码 JWT
        payload = jwt.decode(
            token,
            settings.JWT_SECRET_KEY,
            algorithms=[settings.JWT_ALGORITHM]
        )
        user_id: str = payload.get("sub")
        if user_id is None:
            raise credentials_exception
    except JWTError:
        raise credentials_exception

    # 查询数据库
    # 注意: payload["sub"] 存的是 user_id (int)
    stmt = select(User).where(User.id == int(user_id))
    result = await db.execute(stmt)
    user = result.scalar_one_or_none()

    if user is None:
        raise credentials_exception

    # 检查用户是否被禁用 (可选)
    if not user.is_active:
        raise HTTPException(status_code=400, detail="用户已被禁用")

    return user
