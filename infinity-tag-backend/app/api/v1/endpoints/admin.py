"""
管理员 API 端点
"""
from typing import Optional
from fastapi import APIRouter, Depends, HTTPException, status, Query
from sqlalchemy.ext.asyncio import AsyncSession
from jose import jwt, JWTError
from app.api import deps
from app.services.admin_service import AdminService
from app.services.device_service import DeviceService
from app.schemas.admin import (
    AdminLoginRequest,
    AdminLoginResponse,
    AdminUserListResponse,
    AdminUserDetailResponse,
    AdminUserResponse,
    AdminStatsResponse
)
from app.schemas.device import (
    DeviceCreateRequest,
    DeviceCreateResponse,
    DeviceBatchImportRequest,
    DeviceBatchImportResponse,
    DeviceResponse,
    DeviceListResponse
)
from app.config import settings

router = APIRouter()


def verify_admin_token(token: str = Depends(deps.oauth2_scheme)) -> str:
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


@router.post("/login", response_model=AdminLoginResponse)
async def admin_login(
    request: AdminLoginRequest,
    db: AsyncSession = Depends(deps.get_db)
):
    """管理员登录"""
    service = AdminService(db)
    return await service.login(request)


@router.get("/stats", response_model=AdminStatsResponse)
async def get_stats(
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(verify_admin_token)
):
    """获取统计数据"""
    service = AdminService(db)
    return await service.get_stats()


@router.get("/devices", response_model=DeviceListResponse)
async def get_devices(
    page: int = Query(1, ge=1),
    page_size: int = Query(20, ge=1, le=100),
    status: Optional[str] = Query(None),
    batch_name: Optional[str] = Query(None),
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(verify_admin_token)
):
    """获取设备列表"""
    service = DeviceService(db)
    return await service.get_device_list(
        page=page,
        page_size=page_size,
        status=status,
        batch_name=batch_name
    )


@router.post("/devices", response_model=DeviceCreateResponse)
async def create_device(
    request: DeviceCreateRequest,
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(verify_admin_token)
):
    """录入单个设备（输入UUID，自动计算设备码和初始密码）"""
    service = DeviceService(db)
    return await service.create_device(request)


@router.post("/devices/batch", response_model=DeviceBatchImportResponse)
async def batch_import_devices(
    request: DeviceBatchImportRequest,
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(verify_admin_token)
):
    """批量导入设备"""
    service = DeviceService(db)
    return await service.batch_import(request)


@router.get("/devices/{device_id}", response_model=DeviceResponse)
async def get_device(
    device_id: int,
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(verify_admin_token)
):
    """获取设备详情"""
    service = DeviceService(db)
    return await service.get_device(device_id)


@router.put("/devices/{device_id}/disable", response_model=DeviceResponse)
async def disable_device(
    device_id: int,
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(verify_admin_token)
):
    """禁用设备"""
    service = DeviceService(db)
    return await service.disable_device(device_id)


@router.put("/devices/{device_id}/reset", response_model=DeviceResponse)
async def reset_device(
    device_id: int,
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(verify_admin_token)
):
    """重置设备（解除激活，恢复初始密码有效）"""
    service = DeviceService(db)
    return await service.reset_device(device_id)


@router.delete("/devices/{device_id}")
async def delete_device(
    device_id: int,
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(verify_admin_token)
):
    """删除设备"""
    service = DeviceService(db)
    await service.delete_device(device_id)
    return {"success": True, "message": "设备已删除"}


@router.get("/users", response_model=AdminUserListResponse)
async def get_users(
    page: int = Query(1, ge=1),
    page_size: int = Query(20, ge=1, le=100),
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(verify_admin_token)
):
    """获取用户列表"""
    service = AdminService(db)
    return await service.get_user_list(page=page, page_size=page_size)


@router.get("/users/{user_id}", response_model=AdminUserDetailResponse)
async def get_user_detail(
    user_id: int,
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(verify_admin_token)
):
    """获取用户详情"""
    service = AdminService(db)
    return await service.get_user_detail(user_id)


@router.put("/users/{user_id}/disable", response_model=AdminUserResponse)
async def disable_user(
    user_id: int,
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(verify_admin_token)
):
    """禁用用户"""
    service = AdminService(db)
    return await service.disable_user(user_id)
