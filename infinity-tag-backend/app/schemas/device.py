"""
设备相关的请求/响应模型 - 设备即用户架构
保留 ESP32 设备端需要的 schema
"""
from pydantic import BaseModel
from typing import Optional
from datetime import datetime


class DeviceStatusResponse(BaseModel):
    """ESP32 查询设备状态响应"""
    device_code: str
    status: str
    activated_at: Optional[datetime] = None
