"""
设备相关的请求/响应模型
"""
from pydantic import BaseModel, Field
from typing import Optional, List
from datetime import datetime


class DeviceStatusResponse(BaseModel):
    """ESP32 查询设备状态响应"""
    activated: bool
    activated_at: Optional[datetime] = None


class DeviceCreateRequest(BaseModel):
    """管理员录入设备请求（输入UUID，自动计算设备码和初始密码）"""
    uuid: str = Field(..., min_length=1, description="ESP32芯片UUID")
    batch_name: Optional[str] = Field(None, max_length=100, description="批次名称")
    notes: Optional[str] = Field(None, description="备注")


class DeviceCreateResponse(BaseModel):
    """管理员录入设备响应"""
    id: int
    device_code: str = Field(..., description="6位设备码")
    init_password: str = Field(..., description="6位初始密码（明文，仅此次返回）")
    uuid: str
    batch_name: Optional[str] = None
    created_at: datetime

    class Config:
        from_attributes = True


class DeviceBatchImportRequest(BaseModel):
    """批量导入设备请求"""
    uuids: List[str] = Field(..., min_length=1, description="UUID列表")
    batch_name: Optional[str] = Field(None, max_length=100, description="批次名称")


class DeviceBatchImportResponse(BaseModel):
    """批量导入设备响应"""
    success_count: int
    failed_count: int
    devices: List["DeviceCreateResponse"]
    errors: List[str]


class DeviceResponse(BaseModel):
    """设备详情响应"""
    id: int
    device_code: str
    uuid: str
    status: str
    user_id: Optional[int] = None
    batch_name: Optional[str] = None
    created_at: datetime
    activated_at: Optional[datetime] = None
    notes: Optional[str] = None

    class Config:
        from_attributes = True


class DeviceListResponse(BaseModel):
    """设备列表响应"""
    total: int
    page: int
    page_size: int
    devices: List[DeviceResponse]


class DeviceUpdateRequest(BaseModel):
    """更新设备请求"""
    batch_name: Optional[str] = Field(None, max_length=100)
    notes: Optional[str] = None
