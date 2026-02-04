from pydantic import BaseModel, Field
from typing import Optional
from datetime import datetime


class DeviceActivateRequest(BaseModel):
    """新激活流程：验证设备码+初始密码"""
    device_code: str = Field(..., min_length=6, max_length=6, description="6位设备码")
    init_password: str = Field(..., min_length=6, max_length=6, description="6位初始密码")


class ActivateResponse(BaseModel):
    """激活响应 - 需要设置密码"""
    success: bool = True
    requires_password_setup: bool = True
    temp_token: str = Field(..., description="临时token，仅用于设置密码")
    device_code: str


class SetPasswordRequest(BaseModel):
    """首次设置用户密码"""
    new_password: str = Field(..., min_length=6, description="用户自定义密码")


class SetPasswordResponse(BaseModel):
    """设置密码响应"""
    access_token: str
    token_type: str = "bearer"
    device_id: str


class LoginRequest(BaseModel):
    """登录请求"""
    device_code: str = Field(..., description="设备码")
    password: str = Field(..., description="用户密码")


class LoginResponse(BaseModel):
    """登录响应 - 仅返回 Token"""
    access_token: str
    token_type: str = "bearer"


class UserResponse(BaseModel):
    id: int
    device_id: str
    password_set: bool = False
    activated_at: Optional[datetime] = None
    last_login_at: Optional[datetime] = None

    class Config:
        from_attributes = True
