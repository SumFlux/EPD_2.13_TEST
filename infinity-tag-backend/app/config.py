"""
应用配置管理模块
支持环境变量和 .env 文件配置
"""
from typing import List, Optional
from pydantic_settings import BaseSettings
from pydantic import model_validator
from functools import lru_cache
import os


class Settings(BaseSettings):
    """应用配置（支持环境变量和 .env 文件）"""

    # ====================================
    # 应用基础配置
    # ====================================
    APP_NAME: str = "Infinity Tag Backend"
    APP_VERSION: str = "1.0.0"
    DEBUG: bool = False
    ENVIRONMENT: str = "production"

    # ====================================
    # 数据库配置
    # ====================================
    MYSQL_HOST: str = "localhost"
    MYSQL_PORT: int = 3306
    MYSQL_USER: str = "infinitytag"
    MYSQL_PASSWORD: str
    MYSQL_ROOT_PASSWORD: Optional[str] = None
    MYSQL_DATABASE: str = "infinity_tag"

    @property
    def DATABASE_URL(self) -> str:
        """构建数据库连接 URL"""
        return (
            f"mysql+aiomysql://{self.MYSQL_USER}:{self.MYSQL_PASSWORD}"
            f"@{self.MYSQL_HOST}:{self.MYSQL_PORT}/{self.MYSQL_DATABASE}"
        )

    # ====================================
    # Redis 配置
    # ====================================
    REDIS_HOST: str = "localhost"
    REDIS_PORT: int = 6379
    REDIS_DB: int = 0
    REDIS_PASSWORD: Optional[str] = None

    @property
    def REDIS_URL(self) -> str:
        """构建 Redis 连接 URL"""
        if self.REDIS_PASSWORD:
            return f"redis://:{self.REDIS_PASSWORD}@{self.REDIS_HOST}:{self.REDIS_PORT}/{self.REDIS_DB}"
        return f"redis://{self.REDIS_HOST}:{self.REDIS_PORT}/{self.REDIS_DB}"

    # ====================================
    # AI 服务配置（可动态修改）
    # ====================================
    AI_API_KEY: str  # 必须在环境变量中配置
    AI_MODEL_NAME: str = "doubao-seed-1-6-250615"
    AI_BASE_URL: str = "https://www.dmxapi.cn/v1"
    AI_TIMEOUT: int = 30  # 请求超时时间(秒)
    AI_MAX_RETRIES: int = 2  # 失败重试次数

    # ====================================
    # JWT 配置
    # ====================================
    JWT_SECRET_KEY: str
    JWT_ALGORITHM: str = "HS256"
    JWT_EXPIRATION_HOURS: int = 168  # 7天

    # ====================================
    # 安全配置
    # ====================================
    FORCE_HTTPS: bool = True
    ALLOWED_HOSTS: List[str] = ["infinitytag.app", "api.infinitytag.app"]
    HMAC_SECRET_LENGTH: int = 32
    PASSWORD_MIN_LENGTH: int = 8

    # ====================================
    # 设备码生成盐值配置
    # ====================================
    DEVICE_CODE_SALT: str  # 设备码生成盐（必须配置）
    INIT_PWD_SALT: str     # 初始密码生成盐（必须配置）

    # ====================================
    # 管理员配置
    # ====================================
    ADMIN_USERNAME: str = "admin"
    ADMIN_PASSWORD_HASH: str  # 管理员密码哈希（必须配置）
    ADMIN_JWT_EXPIRATION_HOURS: int = 24  # 管理员 token 24小时过期

    # ====================================
    # 限流配置
    # ====================================
    RATE_LIMIT_PER_MINUTE: int = 60
    RATE_LIMIT_PER_HOUR: int = 1000

    # ====================================
    # 黄历配置
    # ====================================
    ALMANAC_CACHE_DAYS: int = 5
    ALMANAC_HISTORY_DAYS: int = 30

    # ====================================
    # CORS 配置
    # ====================================
    CORS_ORIGINS: List[str] = [
        "https://infinitytag.app",
        "https://www.infinitytag.app"
    ]

    # ====================================
    # 静态资源配置
    # ====================================
    BASE_DIR: str = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    ASSETS_DIR: str = os.path.join(BASE_DIR, "assets")
    FONT_DIR: str = os.path.join(ASSETS_DIR, "fonts")

    class Config:
        env_file = ".env"
        case_sensitive = True

    @model_validator(mode='after')
    def validate_required_secrets(self) -> 'Settings':
        """启动时校验必填的安全配置"""
        missing = []
        if not getattr(self, 'DEVICE_CODE_SALT', None):
            missing.append('DEVICE_CODE_SALT')
        if not getattr(self, 'INIT_PWD_SALT', None):
            missing.append('INIT_PWD_SALT')
        if not getattr(self, 'ADMIN_PASSWORD_HASH', None):
            missing.append('ADMIN_PASSWORD_HASH')

        if missing:
            raise ValueError(
                f"缺少必填的安全配置: {', '.join(missing)}。"
                f"请在 .env 文件中配置这些值。"
            )
        return self


@lru_cache()
def get_settings() -> Settings:
    """获取配置单例（带缓存）"""
    return Settings()


# 导出全局配置实例
settings = get_settings()
