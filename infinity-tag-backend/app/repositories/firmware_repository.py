from typing import Optional
from sqlalchemy import select, desc
from sqlalchemy.ext.asyncio import AsyncSession
from app.models.firmware import Firmware


class FirmwareRepository:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def get_latest_version(self) -> Optional[Firmware]:
        """获取最新激活的固件版本"""
        query = select(Firmware)\
            .where(Firmware.is_active == True)\
            .order_by(desc(Firmware.version_code))\
            .limit(1)
        result = await self.db.execute(query)
        return result.scalar_one_or_none()

    async def get_by_version_code(self, version_code: int) -> Optional[Firmware]:
        """根据版本代码获取固件"""
        query = select(Firmware).where(Firmware.version_code == version_code)
        result = await self.db.execute(query)
        return result.scalar_one_or_none()

    async def create(self, firmware_data: dict) -> Firmware:
        """创建固件记录"""
        firmware = Firmware(**firmware_data)
        self.db.add(firmware)
        await self.db.commit()
        await self.db.refresh(firmware)
        return firmware
