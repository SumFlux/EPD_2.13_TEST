import re
import hashlib
import os
import shutil
from typing import Tuple, Optional
from fastapi import UploadFile, HTTPException, status
from sqlalchemy.ext.asyncio import AsyncSession
from app.repositories.firmware_repository import FirmwareRepository
from app.schemas.firmware import FirmwareResponse, FirmwareCheckResponse, FirmwareCreate
import asyncio
from concurrent.futures import ThreadPoolExecutor


class OTAService:
    # 固件存储路径
    FIRMWARE_DIR = "data/firmwares"

    def __init__(self, db: AsyncSession):
        self.repo = FirmwareRepository(db)

    @staticmethod
    def parse_version(version_str: str) -> int:
        """
        解析版本号 A.B.C.D -> int
        规则: 4位数字 (0-99)
        """
        pattern = r"^(\d{1,2})\.(\d{1,2})\.(\d{1,2})\.(\d{1,2})$"
        match = re.match(pattern, version_str)
        if not match:
            raise ValueError("版本号格式错误，必须为 A.B.C.D (每位0-99)")

        parts = [int(p) for p in match.groups()]
        
        # 转换为整数: A*1000000 + B*10000 + C*100 + D
        version_code = parts[0] * 1000000 + parts[1] * 10000 + parts[2] * 100 + parts[3]
        return version_code

    @staticmethod
    def _calculate_checksum_sync(file_path: str) -> str:
        """同步计算文件 SHA-256 (CPU密集型)"""
        sha256_hash = hashlib.sha256()
        with open(file_path, "rb") as f:
            for byte_block in iter(lambda: f.read(4096), b""):
                sha256_hash.update(byte_block)
        return sha256_hash.hexdigest()

    async def calculate_checksum(self, file_path: str) -> str:
        """
        异步计算文件 SHA-256
        使用 run_in_executor 在线程池中运行，避免阻塞主事件循环
        """
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._calculate_checksum_sync, file_path)

    async def upload_firmware(self, file: UploadFile, version_str: str, description: Optional[str] = None) -> FirmwareResponse:
        """
        上传固件
        1. 验证版本号格式
        2. 验证版本号是否大于当前最新版
        3. 保存文件并计算 Checksum
        4. 写入数据库
        """
        try:
            new_version_code = self.parse_version(version_str)
        except ValueError as e:
            raise HTTPException(status_code=400, detail=str(e))

        # 检查最新版本
        latest_firmware = await self.repo.get_latest_version()
        if latest_firmware:
            if new_version_code <= latest_firmware.version_code:
                raise HTTPException(
                    status_code=400, 
                    detail=f"版本号必须大于当前最新版 ({latest_firmware.version_str})"
                )

        # 检查版本号是否已存在 (理论上上面的检查已经覆盖了，但为了健壮性)
        existing = await self.repo.get_by_version_code(new_version_code)
        if existing:
            raise HTTPException(status_code=400, detail="该版本号已存在")

        # 确保目录存在 (延迟到上传时才检查/创建)
        if not os.path.exists(self.FIRMWARE_DIR):
            os.makedirs(self.FIRMWARE_DIR)

        # 保存文件
        filename = f"firmware_v{version_str}.bin"
        file_path = os.path.join(self.FIRMWARE_DIR, filename).replace("\\", "/")
        
        try:
            with open(file_path, "wb") as buffer:
                shutil.copyfileobj(file.file, buffer)
        except Exception as e:
            raise HTTPException(status_code=500, detail=f"文件保存失败: {str(e)}")

        # 计算 Checksum
        checksum = await self.calculate_checksum(file_path)

        # 创建记录
        create_data = {
            "version_code": new_version_code,
            "version_str": version_str,
            "file_path": file_path,
            "checksum": checksum,
            "description": description,
            "is_active": True
        }

        firmware = await self.repo.create(create_data)
        return FirmwareResponse.model_validate(firmware)

    async def check_update(self, current_version_str: str) -> FirmwareCheckResponse:
        """检查更新"""
        try:
            current_code = self.parse_version(current_version_str)
        except ValueError:
            # 如果客户端传来的版本号格式不对，假设是很旧的版本
            current_code = 0
            
        latest_firmware = await self.repo.get_latest_version()
        
        if not latest_firmware:
            return FirmwareCheckResponse(has_update=False)

        if latest_firmware.version_code > current_code:
            # 构造下载链接
            # 注意：实际部署时可能需要反向代理或OSS，这里假设本地服务
            download_url = f"/api/v1/ota/download/{latest_firmware.version_str}"
            
            return FirmwareCheckResponse(
                has_update=True,
                latest_version=latest_firmware.version_str,
                download_url=download_url,
                checksum=latest_firmware.checksum,
                description=latest_firmware.description
            )
        
        return FirmwareCheckResponse(has_update=False)

    async def get_download_path(self, version_str: str) -> str:
        """获取固件下载路径"""
        try:
            version_code = self.parse_version(version_str)
        except ValueError:
            raise HTTPException(status_code=404, detail="版本号格式错误")
            
        firmware = await self.repo.get_by_version_code(version_code)
        if not firmware or not os.path.exists(firmware.file_path):
            raise HTTPException(status_code=404, detail="固件文件不存在")
            
        return firmware.file_path
