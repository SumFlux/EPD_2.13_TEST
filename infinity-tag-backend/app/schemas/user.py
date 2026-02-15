"""
用户与档案 Schema
"""
from typing import Optional
from pydantic import BaseModel, Field


# ====================================
# 认证相关
# ====================================
class DeviceActivateRequest(BaseModel):
    """设备激活请求"""
    device_code: str = Field(..., min_length=6, max_length=6, description="6位设备短码")
    device_uuid: Optional[str] = Field(None, max_length=64, description="设备硬件唯一ID (可选)")


class TokenResponse(BaseModel):
    """JWT Token 响应"""
    access_token: str
    token_type: str = "bearer"
    expires_in: int


# ====================================
# 档案相关
# ====================================
class UserProfileBase(BaseModel):
    """档案基础字段"""
    nickname: str = Field(..., min_length=1, max_length=32, description="昵称")
    gender: int = Field(1, ge=0, le=1, description="性别: 1男 0女")

    # 生辰
    birth_year: int = Field(..., ge=1900, le=2100, description="出生年")
    birth_month: int = Field(..., ge=1, le=12, description="出生月")
    birth_day: int = Field(..., ge=1, le=31, description="出生日")
    birth_hour: int = Field(-1, ge=-1, le=23, description="出生时辰 (0-23, -1为未知)")

    is_lunar: bool = Field(False, description="是否为农历生日")
    occupation: Optional[str] = Field(None, max_length=32, description="职业/身份 (如 程序员)")
    notes: Optional[str] = Field(None, description="备注")


class UserProfileCreate(UserProfileBase):
    """创建档案请求"""
    pass


class UserProfileUpdate(UserProfileBase):
    """更新档案请求"""
    pass


class UserProfileResponse(UserProfileBase):
    """档案响应"""
    user_id: int

    class Config:
        from_attributes = True


class UserResponse(BaseModel):
    """用户完整信息响应（包含设备信息）"""
    id: int
    device_code: str
    uuid: str
    status: str
    password_set: bool
    activated_at: Optional[str] = None
    last_login_at: Optional[str] = None
    profile: Optional[UserProfileResponse] = None

    class Config:
        from_attributes = True
