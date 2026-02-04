"""
设备数据访问层
"""
from typing import Optional, List, Tuple
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, func
from app.models.device import Device, DeviceStatus
from datetime import datetime, timezone


class DeviceRepository:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def get_by_id(self, device_id: int) -> Optional[Device]:
        """根据ID获取设备"""
        stmt = select(Device).where(Device.id == device_id)
        result = await self.session.execute(stmt)
        return result.scalar_one_or_none()

    async def get_by_device_code(self, device_code: str) -> Optional[Device]:
        """根据设备码获取设备"""
        stmt = select(Device).where(Device.device_code == device_code)
        result = await self.session.execute(stmt)
        return result.scalar_one_or_none()

    async def get_by_uuid(self, uuid: str) -> Optional[Device]:
        """根据UUID获取设备"""
        stmt = select(Device).where(Device.uuid == uuid)
        result = await self.session.execute(stmt)
        return result.scalar_one_or_none()

    async def create(self, device_data: dict) -> Device:
        """创建设备"""
        device = Device(**device_data)
        self.session.add(device)
        await self.session.commit()
        await self.session.refresh(device)
        return device

    async def update(self, device: Device, update_data: dict) -> Device:
        """更新设备"""
        for key, value in update_data.items():
            setattr(device, key, value)
        await self.session.commit()
        await self.session.refresh(device)
        return device

    async def delete(self, device: Device) -> None:
        """删除设备"""
        await self.session.delete(device)
        await self.session.commit()

    async def activate(self, device: Device, user_id: int) -> Device:
        """激活设备"""
        device.status = DeviceStatus.ACTIVATED.value
        device.user_id = user_id
        device.activated_at = datetime.now(timezone.utc)
        await self.session.commit()
        await self.session.refresh(device)
        return device

    async def disable(self, device: Device) -> Device:
        """禁用设备"""
        device.status = DeviceStatus.DISABLED.value
        await self.session.commit()
        await self.session.refresh(device)
        return device

    async def reset(self, device: Device) -> Device:
        """重置设备（解除激活，恢复初始密码有效）"""
        device.status = DeviceStatus.PENDING.value
        device.user_id = None
        device.activated_at = None
        await self.session.commit()
        await self.session.refresh(device)
        return device

    async def get_list(
        self,
        page: int = 1,
        page_size: int = 20,
        status: Optional[str] = None,
        batch_name: Optional[str] = None
    ) -> Tuple[List[Device], int]:
        """获取设备列表（分页）"""
        stmt = select(Device)

        if status:
            stmt = stmt.where(Device.status == status)
        if batch_name:
            stmt = stmt.where(Device.batch_name == batch_name)

        # 计算总数
        count_stmt = select(func.count()).select_from(stmt.subquery())
        total_result = await self.session.execute(count_stmt)
        total = total_result.scalar() or 0

        # 分页
        stmt = stmt.order_by(Device.created_at.desc())
        stmt = stmt.offset((page - 1) * page_size).limit(page_size)

        result = await self.session.execute(stmt)
        devices = result.scalars().all()

        return list(devices), total

    async def get_stats(self) -> dict:
        """获取设备统计"""
        total_stmt = select(func.count(Device.id))
        pending_stmt = select(func.count(Device.id)).where(
            Device.status == DeviceStatus.PENDING.value
        )
        activated_stmt = select(func.count(Device.id)).where(
            Device.status == DeviceStatus.ACTIVATED.value
        )
        disabled_stmt = select(func.count(Device.id)).where(
            Device.status == DeviceStatus.DISABLED.value
        )

        total = (await self.session.execute(total_stmt)).scalar() or 0
        pending = (await self.session.execute(pending_stmt)).scalar() or 0
        activated = (await self.session.execute(activated_stmt)).scalar() or 0
        disabled = (await self.session.execute(disabled_stmt)).scalar() or 0

        return {
            "total_devices": total,
            "pending_devices": pending,
            "activated_devices": activated,
            "disabled_devices": disabled
        }
