"""
安全工具库
处理密码哈希、JWT Token 和 HMAC 签名
"""
from datetime import datetime, timedelta
from typing import Optional, Any
from passlib.context import CryptContext
from jose import jwt
import hmac
import hashlib
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


class SecurityService:
    """安全服务封装"""

    @staticmethod
    def generate_hmac_signature(
        payload: str,
        secret: str,
        timestamp: int
    ) -> str:
        """生成 HMAC-SHA256 签名"""
        message = f"{payload}:{timestamp}"
        signature = hmac.new(
            secret.encode(),
            message.encode(),
            hashlib.sha256
        ).hexdigest()
        return signature

    @staticmethod
    def verify_hmac_signature(
        payload: str,
        signature: str,
        secret: str,
        timestamp: int,
        tolerance_seconds: int = 300
    ) -> bool:
        """验证签名（含时间戳检查，防重放攻击）"""
        # 检查时间戳
        current_timestamp = int(datetime.utcnow().timestamp())
        if abs(current_timestamp - timestamp) > tolerance_seconds:
            return False

        # 计算期望签名
        expected = SecurityService.generate_hmac_signature(
            payload, secret, timestamp
        )

        # 恒定时间比较（防时序攻击）
        return hmac.compare_digest(signature, expected)
