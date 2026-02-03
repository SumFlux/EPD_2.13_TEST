from pydantic import BaseModel, Field
from typing import Optional

class DeviceActivateRequest(BaseModel):
    device_id: Optional[str] = Field(None, description="设备ID (6位短码)，为空则自动生成")
    password: str = Field(..., min_length=6, description="用户设置的密码")

class LoginRequest(BaseModel):
    device_id: str = Field(..., description="设备ID")
    password: str = Field(..., description="密码")

class LoginResponse(BaseModel):
    """登录响应 - 仅返回 Token"""
    access_token: str
    token_type: str = "bearer"

class ActivateResponse(BaseModel):
    """激活响应 - 返回 Token 和 设备密钥"""
    access_token: str
    token_type: str = "bearer"
    device_secret: str = Field(..., description="设备HMAC密钥 (仅首次返回，请妥善保存)")
    user_id: int
    device_id: str

# 兼容旧代码的别名，但建议逐步迁移
TokenResponse = ActivateResponse

class UserResponse(BaseModel):
    id: int
    device_id: str
    activated_at: Optional[str]
    last_login_at: Optional[str]

    class Config:
        from_attributes = True
