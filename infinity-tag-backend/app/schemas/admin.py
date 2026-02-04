"""
管理员相关的请求/响应模型
"""
from pydantic import BaseModel, Field
from typing import Optional, List
from datetime import datetime


class AdminLoginRequest(BaseModel):
    """管理员登录请求"""
    username: str = Field(..., min_length=1)
    password: str = Field(..., min_length=1)


class AdminLoginResponse(BaseModel):
    """管理员登录响应"""
    access_token: str
    token_type: str = "bearer"
    expires_in: int = Field(..., description="过期时间（秒）")


class AdminUserResponse(BaseModel):
    """管理员查看的用户信息"""
    id: int
    device_id: str
    password_set: bool
    activated_at: Optional[datetime] = None
    last_login_at: Optional[datetime] = None
    created_at: datetime

    class Config:
        from_attributes = True


class AdminUserListResponse(BaseModel):
    """用户列表响应"""
    total: int
    page: int
    page_size: int
    users: List[AdminUserResponse]


class AdminUserDetailResponse(AdminUserResponse):
    """用户详情响应（含关联设备）"""
    device_code: Optional[str] = None
    device_status: Optional[str] = None


class AdminStatsResponse(BaseModel):
    """统计数据响应"""
    total_devices: int
    pending_devices: int
    activated_devices: int
    disabled_devices: int
    total_users: int
    users_with_password: int


class AdminLogResponse(BaseModel):
    """操作日志响应"""
    id: int
    action: str
    target_type: str
    target_id: Optional[int] = None
    details: Optional[str] = None
    admin_username: str
    created_at: datetime


class AdminLogListResponse(BaseModel):
    """操作日志列表响应"""
    total: int
    page: int
    page_size: int
    logs: List[AdminLogResponse]
