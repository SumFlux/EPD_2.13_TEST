"""
管理员 API 端点 - 设备即用户架构
"""
from typing import Optional
from fastapi import APIRouter, Depends, Query
from sqlalchemy.ext.asyncio import AsyncSession
from app.api import deps
from app.services.admin_service import AdminService
from app.schemas.admin import (
    AdminLoginRequest,
    AdminLoginResponse,
    AdminUserListResponse,
    AdminUserDetailResponse,
    AdminUserResponse,
    AdminStatsResponse,
    DeviceCreateRequest,
    DeviceCreateResponse,
    DeviceBatchImportRequest,
    DeviceBatchImportResponse
)

router = APIRouter()


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
    _: str = Depends(deps.verify_admin_token)
):
    """获取统计数据"""
    service = AdminService(db)
    return await service.get_stats()


@router.get("/devices", response_model=AdminUserListResponse)
async def get_devices(
    page: int = Query(1, ge=1),
    page_size: int = Query(20, ge=1, le=100),
    status: Optional[str] = Query(None),
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(deps.verify_admin_token)
):
    """获取设备列表（查询 User 表）"""
    service = AdminService(db)
    return await service.get_device_list(
        page=page,
        page_size=page_size,
        status_filter=status
    )


@router.post("/devices", response_model=DeviceCreateResponse)
async def create_device(
    request: DeviceCreateRequest,
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(deps.verify_admin_token)
):
    """创建设备（创建 User 记录）"""
    service = AdminService(db)
    return await service.create_device(request)


@router.post("/devices/batch", response_model=DeviceBatchImportResponse)
async def batch_import_devices(
    request: DeviceBatchImportRequest,
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(deps.verify_admin_token)
):
    """批量导入设备（批量创建 User 记录）"""
    service = AdminService(db)
    return await service.batch_import_devices(request)


@router.get("/devices/{device_code}", response_model=AdminUserDetailResponse)
async def get_device(
    device_code: str,
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(deps.verify_admin_token)
):
    """获取设备详情（通过 device_code 查询）"""
    service = AdminService(db)
    return await service.get_device_detail(device_code)


@router.put("/devices/{device_code}/disable", response_model=AdminUserResponse)
async def disable_device(
    device_code: str,
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(deps.verify_admin_token)
):
    """禁用设备（更新 User.status）"""
    service = AdminService(db)
    return await service.disable_device(device_code)
