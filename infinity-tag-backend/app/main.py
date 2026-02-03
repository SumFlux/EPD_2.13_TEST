"""
Infinity Tag Backend - FastAPI 应用入口
"""
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from app.config import settings

# 创建 FastAPI 应用实例
app = FastAPI(
    title=settings.APP_NAME,
    version=settings.APP_VERSION,
    description="Infinity Tag（无止便签）Web 后端 API",
    docs_url="/docs" if settings.DEBUG else None,
    redoc_url="/redoc" if settings.DEBUG else None,
)

# ====================================
# CORS 中间件配置
# ====================================
app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.CORS_ORIGINS,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


# ====================================
# 健康检查端点
# ====================================
@app.get("/health", tags=["系统"])
async def health_check():
    """健康检查接口"""
    return {
        "status": "ok",
        "app": settings.APP_NAME,
        "version": settings.APP_VERSION,
        "environment": settings.ENVIRONMENT
    }


@app.get("/", tags=["系统"])
async def root():
    """根路径"""
    return {
        "message": "Infinity Tag Backend API",
        "version": settings.APP_VERSION,
        "docs": "/docs" if settings.DEBUG else "文档已禁用"
    }


# ====================================
# 全局异常处理
# ====================================
@app.exception_handler(Exception)
async def global_exception_handler(request, exc):
    """全局异常处理器"""
    return JSONResponse(
        status_code=500,
        content={
            "success": False,
            "error": "Internal Server Error",
            "message": str(exc) if settings.DEBUG else "服务器内部错误"
        }
    )


# ====================================
# 启动和关闭事件
# ====================================
@app.on_event("startup")
async def startup_event():
    """应用启动时执行"""
    print(f"[INFO] {settings.APP_NAME} v{settings.APP_VERSION} 启动成功！")
    print(f"[ENV] 环境: {settings.ENVIRONMENT}")
    print(f"[SEC] HTTPS 强制: {'启用' if settings.FORCE_HTTPS else '禁用'}")
    print(f"[DB]  数据库: {settings.MYSQL_HOST}:{settings.MYSQL_PORT}/{settings.MYSQL_DATABASE}")
    print(f"[RED] Redis: {settings.REDIS_HOST}:{settings.REDIS_PORT}")


@app.on_event("shutdown")
async def shutdown_event():
    """应用关闭时执行"""
    print(f"[INFO] {settings.APP_NAME} 正在关闭...")


# ====================================
# 导入路由
# ====================================
from app.api.v1.api import api_router
app.include_router(api_router, prefix="/api/v1")


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "app.main:app",
        host="0.0.0.0",
        port=8000,
        reload=settings.DEBUG
    )
