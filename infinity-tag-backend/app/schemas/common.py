"""
通用 Schema 定义
"""
from typing import Generic, TypeVar, Optional
from pydantic import BaseModel, Field

T = TypeVar("T")


class ResponseBase(BaseModel, Generic[T]):
    """
    通用 API 响应包装器
    """
    success: bool = Field(default=True, description="请求是否成功")
    message: str = Field(default="ok", description="提示消息")
    data: Optional[T] = Field(default=None, description="业务数据")
    error_code: Optional[int] = Field(default=None, description="错误码 (仅失败时返回)")

    class Config:
        from_attributes = True


class PageParams(BaseModel):
    """
    分页请求参数
    """
    page: int = Field(default=1, ge=1, description="页码")
    size: int = Field(default=10, ge=1, le=100, description="每页数量")


class PageResponse(BaseModel, Generic[T]):
    """
    分页响应数据
    """
    items: list[T]
    total: int
    page: int
    size: int
    pages: int
