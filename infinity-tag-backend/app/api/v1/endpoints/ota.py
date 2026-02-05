"""
OTA 升级相关 API
"""
from typing import Optional
from fastapi import APIRouter, Depends, UploadFile, File, Form
from fastapi.responses import FileResponse
from sqlalchemy.ext.asyncio import AsyncSession
from app.api import deps
from app.services.ota_service import OTAService
from app.schemas.firmware import FirmwareResponse, FirmwareCheckResponse

router = APIRouter()


@router.post("/upload", response_model=FirmwareResponse)
async def upload_firmware(
    file: UploadFile = File(...),
    version: str = Form(..., description="版本号，如 1.0.0.1"),
    description: Optional[str] = Form(None),
    db: AsyncSession = Depends(deps.get_db),
    _: str = Depends(deps.verify_admin_token)
):
    """
    上传新固件 (Admin Only)
    """
    service = OTAService(db)
    return await service.upload_firmware(file, version, description)


@router.get("/check", response_model=FirmwareCheckResponse)
async def check_update(
    version: str,
    db: AsyncSession = Depends(deps.get_db),
    # 设备端检查更新通常也需要鉴权，这里和下载保持一致，要求有效的 User/Device Token
    _: deps.User = Depends(deps.get_current_user)
):
    """
    检查更新 (Device)
    传入当前版本号，返回是否有更新
    """
    service = OTAService(db)
    return await service.check_update(version)


@router.get("/download/{version_str}")
async def download_firmware(
    version_str: str,
    db: AsyncSession = Depends(deps.get_db),
    # 增加鉴权，防止未授权下载
    _: deps.User = Depends(deps.get_current_user)
):
    """
    下载固件文件 (Device)
    """
    service = OTAService(db)
    file_path = await service.get_download_path(version_str)
    return FileResponse(
        file_path,
        media_type="application/octet-stream",
        filename=f"firmware_{version_str}.bin"
    )
