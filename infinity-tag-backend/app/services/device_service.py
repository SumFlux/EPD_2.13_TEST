"""
设备业务逻辑服务
"""
from typing import Optional
from sqlalchemy.ext.asyncio import AsyncSession
from fastapi import HTTPException, status
from app.repositories.device_repository import DeviceRepository
from app.models.device import Device, DeviceStatus
from app.utils.device_code_gen import generate_device_code, generate_init_password
from app.core.security import get_password_hash, verify_password
from app.schemas.device import (
    DeviceCreateRequest,
    DeviceCreateResponse,
    DeviceBatchImportRequest,
    DeviceBatchImportResponse,
    DeviceResponse,
    DeviceListResponse,
    DeviceStatusResponse
)


class DeviceService:
    def __init__(self, db: AsyncSession):
        self.device_repo = DeviceRepository(db)

    async def create_device(self, request: DeviceCreateRequest) -> DeviceCreateResponse:
        """管理员录入设备（输入UUID，自动计算设备码和初始密码）"""
        # 检查UUID是否已存在
        existing = await self.device_repo.get_by_uuid(request.uuid)
        if existing:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="该UUID已被录入"
            )

        # 生成设备码和初始密码
        device_code = generate_device_code(request.uuid)
        init_password = generate_init_password(request.uuid)

        # 检查设备码是否已存在（理论上不应该，但安全起见）
        existing_code = await self.device_repo.get_by_device_code(device_code)
        if existing_code:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="设备码冲突，请联系管理员"
            )

        # 创建设备
        device_data = {
            "device_code": device_code,
            "init_password_hash": get_password_hash(init_password),
            "uuid": request.uuid,
            "batch_name": request.batch_name,
            "notes": request.notes,
            "status": DeviceStatus.PENDING.value
        }

        device = await self.device_repo.create(device_data)

        return DeviceCreateResponse(
            id=device.id,
            device_code=device_code,
            init_password=init_password,  # 明文返回，仅此次
            uuid=device.uuid,
            batch_name=device.batch_name,
            created_at=device.created_at
        )

    async def batch_import(
        self, request: DeviceBatchImportRequest
    ) -> DeviceBatchImportResponse:
        """批量导入设备"""
        success_devices = []
        errors = []

        for uuid in request.uuids:
            try:
                device_request = DeviceCreateRequest(
                    uuid=uuid,
                    batch_name=request.batch_name
                )
                device_response = await self.create_device(device_request)
                success_devices.append(device_response)
            except HTTPException as e:
                errors.append(f"UUID {uuid}: {e.detail}")
            except Exception as e:
                errors.append(f"UUID {uuid}: {str(e)}")

        return DeviceBatchImportResponse(
            success_count=len(success_devices),
            failed_count=len(errors),
            devices=success_devices,
            errors=errors
        )

    async def get_device(self, device_id: int) -> DeviceResponse:
        """获取设备详情"""
        device = await self.device_repo.get_by_id(device_id)
        if not device:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="设备不存在"
            )
        return DeviceResponse.model_validate(device)

    async def get_device_list(
        self,
        page: int = 1,
        page_size: int = 20,
        status: Optional[str] = None,
        batch_name: Optional[str] = None
    ) -> DeviceListResponse:
        """获取设备列表"""
        devices, total = await self.device_repo.get_list(
            page=page,
            page_size=page_size,
            status=status,
            batch_name=batch_name
        )

        return DeviceListResponse(
            total=total,
            page=page,
            page_size=page_size,
            devices=[DeviceResponse.model_validate(d) for d in devices]
        )

    async def disable_device(self, device_id: int) -> DeviceResponse:
        """禁用设备"""
        device = await self.device_repo.get_by_id(device_id)
        if not device:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="设备不存在"
            )

        device = await self.device_repo.disable(device)
        return DeviceResponse.model_validate(device)

    async def reset_device(self, device_id: int) -> DeviceResponse:
        """重置设备（解除激活，恢复初始密码有效）"""
        device = await self.device_repo.get_by_id(device_id)
        if not device:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="设备不存在"
            )

        device = await self.device_repo.reset(device)
        return DeviceResponse.model_validate(device)

    async def delete_device(self, device_id: int) -> None:
        """删除设备"""
        device = await self.device_repo.get_by_id(device_id)
        if not device:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="设备不存在"
            )

        await self.device_repo.delete(device)

    async def get_device_status(self, device_code: str) -> DeviceStatusResponse:
        """ESP32 查询设备激活状态"""
        device = await self.device_repo.get_by_device_code(device_code)
        if not device:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="设备不存在"
            )

        return DeviceStatusResponse(
            activated=device.status == DeviceStatus.ACTIVATED.value,
            activated_at=device.activated_at
        )

    async def verify_init_password(
        self, device_code: str, init_password: str
    ) -> Device:
        """验证设备码和初始密码（用于激活流程）"""
        device = await self.device_repo.get_by_device_code(device_code)
        if not device:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="无效的设备码"
            )

        if device.status == DeviceStatus.ACTIVATED.value:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="该设备已被激活"
            )

        if device.status == DeviceStatus.DISABLED.value:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="该设备已被禁用"
            )

        if not verify_password(init_password, device.init_password_hash):
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="初始密码错误"
            )

        return device
