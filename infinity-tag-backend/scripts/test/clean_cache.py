"""
工具脚本: 清除黄历缓存
直接连接数据库删除 almanac_history 表的所有数据
"""
import asyncio
import sys
import os
from sqlalchemy import text
from sqlalchemy.ext.asyncio import create_async_engine

# 将项目根目录加入路径，以便导入配置
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "../../")))
from app.config import settings

async def clean_cache():
    print(f"正在连接数据库: {settings.MYSQL_HOST}:{settings.MYSQL_PORT}")

    # 创建临时引擎
    engine = create_async_engine(settings.DATABASE_URL)

    async with engine.begin() as conn:
        try:
            # 执行删除
            await conn.execute(text("DELETE FROM almanac_history"))
            print("✅ 成功清除 almanac_history 表的所有缓存数据")
        except Exception as e:
            print(f"❌ 清除失败: {e}")

    await engine.dispose()

if __name__ == "__main__":
    # Windows 下通常需要设置 selector event loop
    if sys.platform == 'win32':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

    asyncio.run(clean_cache())
