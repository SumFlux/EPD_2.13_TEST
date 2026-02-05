from datetime import datetime
from typing import Optional
from pydantic import BaseModel, ConfigDict


class FirmwareBase(BaseModel):
    version_str: str
    description: Optional[str] = None
    is_active: bool = True


class FirmwareCreate(FirmwareBase):
    version_code: int
    file_path: str
    checksum: str


class FirmwareUpdate(BaseModel):
    is_active: Optional[bool] = None
    description: Optional[str] = None


class FirmwareResponse(FirmwareBase):
    id: int
    version_code: int
    file_path: str
    checksum: str
    created_at: datetime
    updated_at: datetime

    model_config = ConfigDict(from_attributes=True)


class FirmwareCheckResponse(BaseModel):
    has_update: bool
    latest_version: Optional[str] = None
    download_url: Optional[str] = None
    checksum: Optional[str] = None
    description: Optional[str] = None
