"""
安全工具库
处理密码哈希和 JWT Token
"""
from datetime import datetime, timedelta
from typing import Optional, Any
from passlib.context import CryptContext
from jose import jwt
from app.config import settings

# 密码哈希上下文
pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")


def verify_password(plain_password: str, hashed_password: str) -> bool:
    """验证密码"""
    return pwd_context.verify(plain_password, hashed_password)


def get_password_hash(password: str) -> str:
    """生成密码哈希"""
    return pwd_context.hash(password)


def create_access_token(subject: Any, expires_delta: Optional[timedelta] = None) -> str:
    """
    创建 JWT Access Token
    :param subject: 主题 (通常是 device_code 或 user_id)
    :param expires_delta: 过期时间差
    """
    if expires_delta:
        expire = datetime.utcnow() + expires_delta
    else:
        expire = datetime.utcnow() + timedelta(hours=settings.JWT_EXPIRATION_HOURS)

    to_encode = {
        "sub": str(subject),
        "exp": expire,
        "type": "access"
    }

    encoded_jwt = jwt.encode(
        to_encode,
        settings.JWT_SECRET_KEY,
        algorithm=settings.JWT_ALGORITHM
    )
    return encoded_jwt
