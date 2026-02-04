"""
Pytest 配置和共享 fixtures
"""
import os
import asyncio
from typing import AsyncGenerator, Generator
import pytest
import pytest_asyncio
from httpx import AsyncClient, ASGITransport
from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession, async_sessionmaker
from sqlalchemy.pool import StaticPool

# 设置测试环境变量（必须在导入 app 之前）
os.environ["MYSQL_PASSWORD"] = "test_password"
os.environ["AI_API_KEY"] = "test_api_key"
os.environ["JWT_SECRET_KEY"] = "test_jwt_secret_key_for_testing_only"
os.environ["DEVICE_CODE_SALT"] = "test_device_code_salt"
os.environ["INIT_PWD_SALT"] = "test_init_pwd_salt"
os.environ["ADMIN_PASSWORD_HASH"] = "$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4.V9s4.YqKjGZ7Hy"
os.environ["DEBUG"] = "true"
os.environ["ENVIRONMENT"] = "test"

from sqlalchemy import event
from app.models.base import Base
from app.core.database import AsyncSessionLocal
from app.api.deps import get_db
from app.main import app


# 使用 SQLite 内存数据库进行测试
TEST_DATABASE_URL = "sqlite+aiosqlite:///:memory:"


@pytest.fixture(scope="session")
def event_loop() -> Generator[asyncio.AbstractEventLoop, None, None]:
    """创建事件循环"""
    loop = asyncio.get_event_loop_policy().new_event_loop()
    yield loop
    loop.close()


@pytest_asyncio.fixture(scope="function")
async def test_engine():
    """创建测试数据库引擎"""
    engine = create_async_engine(
        TEST_DATABASE_URL,
        connect_args={"check_same_thread": False},
        poolclass=StaticPool,
        echo=False
    )

    # SQLite 外键支持
    @event.listens_for(engine.sync_engine, "connect")
    def set_sqlite_pragma(dbapi_connection, connection_record):
        cursor = dbapi_connection.cursor()
        cursor.execute("PRAGMA foreign_keys=ON")
        cursor.close()

    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)

    yield engine

    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.drop_all)

    await engine.dispose()


@pytest_asyncio.fixture(scope="function")
async def test_session(test_engine) -> AsyncGenerator[AsyncSession, None]:
    """创建测试数据库会话"""
    async_session = async_sessionmaker(
        test_engine,
        class_=AsyncSession,
        expire_on_commit=False
    )

    async with async_session() as session:
        yield session
        await session.rollback()


@pytest_asyncio.fixture(scope="function")
async def client(test_engine) -> AsyncGenerator[AsyncClient, None]:
    """创建测试 HTTP 客户端"""
    async_session = async_sessionmaker(
        test_engine,
        class_=AsyncSession,
        expire_on_commit=False
    )

    async def override_get_db() -> AsyncGenerator[AsyncSession, None]:
        async with async_session() as session:
            yield session

    app.dependency_overrides[get_db] = override_get_db

    transport = ASGITransport(app=app)
    async with AsyncClient(transport=transport, base_url="http://test") as ac:
        yield ac

    app.dependency_overrides.clear()


@pytest_asyncio.fixture
async def test_device(test_session: AsyncSession):
    """创建测试设备"""
    from app.models.device import Device, DeviceStatus
    from app.core.security import get_password_hash

    device = Device(
        device_code="ABC123",
        uuid="test-uuid-12345",
        init_password_hash=get_password_hash("123456"),
        status=DeviceStatus.PENDING.value,
        notes="测试设备"
    )
    test_session.add(device)
    await test_session.commit()
    await test_session.refresh(device)
    return device


@pytest_asyncio.fixture
async def activated_device_and_user(test_session: AsyncSession, test_device):
    """创建已激活的设备和用户"""
    from app.models.user import User
    from app.models.device import DeviceStatus
    from app.core.security import get_password_hash
    from datetime import datetime, timezone

    # 创建用户
    user = User(
        device_id=test_device.device_code,
        password_hash=get_password_hash("user_password"),
        device_secret="test_device_secret",
        password_set=True,
        activated_at=datetime.now(timezone.utc)
    )
    test_session.add(user)

    # 更新设备状态
    test_device.status = DeviceStatus.ACTIVATED.value
    test_device.activated_user_id = 1

    await test_session.commit()
    await test_session.refresh(user)
    await test_session.refresh(test_device)

    return {"device": test_device, "user": user}
