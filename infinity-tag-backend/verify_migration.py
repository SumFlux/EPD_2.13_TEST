"""
数据库迁移验证脚本
"""
import asyncio
from sqlalchemy import create_engine, text
from app.config import settings

def verify_pre_migration():
    """迁移前验证"""
    print("=" * 60)
    print("任务 7.1: 检查当前数据库状态")
    print("=" * 60)

    # 使用同步引擎进行验证
    sync_url = settings.DATABASE_URL.replace('mysql+aiomysql', 'mysql+pymysql')
    engine = create_engine(sync_url)

    with engine.connect() as conn:
        # 1. 检查表是否存在
        print("\n1. 检查现有表:")
        result = conn.execute(text("""
            SELECT table_name
            FROM information_schema.tables
            WHERE table_schema = :db_name
            ORDER BY table_name
        """), {"db_name": settings.MYSQL_DATABASE})
        tables = [row[0] for row in result.fetchall()]
        for table in tables:
            print(f"   - {table}")

        # 2. 检查 users 表结构
        print("\n2. Users 表当前字段:")
        result = conn.execute(text("""
            SELECT column_name, data_type, is_nullable, column_key
            FROM information_schema.columns
            WHERE table_schema = :db_name AND table_name = 'users'
            ORDER BY ordinal_position
        """), {"db_name": settings.MYSQL_DATABASE})
        for row in result.fetchall():
            print(f"   - {row[0]}: {row[1]} (nullable={row[2]}, key={row[3]})")

        # 3. 检查 devices 表是否存在
        if 'devices' in tables:
            print("\n3. Devices 表记录数:")
            result = conn.execute(text("SELECT COUNT(*) FROM devices"))
            device_count = result.scalar()
            print(f"   设备总数: {device_count}")

            if device_count > 0:
                print("\n   前5条设备记录:")
                result = conn.execute(text("""
                    SELECT device_code, uuid, status, activated_at
                    FROM devices
                    LIMIT 5
                """))
                for row in result.fetchall():
                    print(f"   - {row[0]}: uuid={row[1]}, status={row[2]}, activated={row[3]}")
        else:
            print("\n3. Devices 表不存在")

        # 4. 检查 users 表记录数
        print("\n4. Users 表记录数:")
        result = conn.execute(text("SELECT COUNT(*) FROM users"))
        user_count = result.scalar()
        print(f"   用户总数: {user_count}")

        if user_count > 0:
            print("\n   前5条用户记录:")
            result = conn.execute(text("""
                SELECT id, device_id, password_set, activated_at
                FROM users
                LIMIT 5
            """))
            for row in result.fetchall():
                print(f"   - ID={row[0]}: device_id={row[1]}, password_set={row[2]}, activated={row[3]}")

        # 5. 检查外键关系
        print("\n5. Users 表外键约束:")
        result = conn.execute(text("""
            SELECT constraint_name, column_name, referenced_table_name, referenced_column_name
            FROM information_schema.key_column_usage
            WHERE table_schema = :db_name
            AND table_name = 'users'
            AND referenced_table_name IS NOT NULL
        """), {"db_name": settings.MYSQL_DATABASE})
        fks = result.fetchall()
        if fks:
            for row in fks:
                print(f"   - {row[0]}: {row[1]} -> {row[2]}.{row[3]}")
        else:
            print("   无外键约束")

    engine.dispose()
    print("\n" + "=" * 60)
    print("数据库状态检查完成")
    print("=" * 60)

def verify_post_migration():
    """迁移后验证"""
    print("\n" + "=" * 60)
    print("任务 7.3: 验证数据迁移正确性")
    print("=" * 60)

    sync_url = settings.DATABASE_URL.replace('mysql+aiomysql', 'mysql+pymysql')
    engine = create_engine(sync_url)

    with engine.connect() as conn:
        # 1. 检查 User 表结构
        print("\n1. Users 表新字段:")
        result = conn.execute(text("""
            SELECT column_name, data_type, is_nullable, column_key
            FROM information_schema.columns
            WHERE table_schema = :db_name AND table_name = 'users'
            AND column_name IN ('device_code', 'init_password_hash', 'uuid', 'status')
            ORDER BY ordinal_position
        """), {"db_name": settings.MYSQL_DATABASE})
        for row in result.fetchall():
            print(f"   - {row[0]}: {row[1]} (nullable={row[2]}, key={row[3]})")

        # 2. 检查记录数
        print("\n2. Users 表记录数:")
        result = conn.execute(text("SELECT COUNT(*) FROM users"))
        user_count = result.scalar()
        print(f"   用户总数: {user_count}")

        # 3. 检查必填字段
        print("\n3. 检查空值记录:")
        result = conn.execute(text("""
            SELECT COUNT(*) FROM users
            WHERE device_code IS NULL OR uuid IS NULL OR init_password_hash IS NULL
        """))
        null_count = result.scalar()
        if null_count == 0:
            print("   [OK] 所有必填字段都有值")
        else:
            print(f"   [ERROR] 发现 {null_count} 条空值记录")

        # 4. 检查唯一约束
        print("\n4. 检查唯一性约束:")
        result = conn.execute(text("""
            SELECT device_code, COUNT(*) as cnt
            FROM users
            GROUP BY device_code
            HAVING COUNT(*) > 1
        """))
        duplicates = result.fetchall()
        if not duplicates:
            print("   [OK] device_code 无重复")
        else:
            print(f"   [ERROR] 发现重复 device_code: {duplicates}")

        result = conn.execute(text("""
            SELECT uuid, COUNT(*) as cnt
            FROM users
            GROUP BY uuid
            HAVING COUNT(*) > 1
        """))
        duplicates = result.fetchall()
        if not duplicates:
            print("   [OK] uuid 无重复")
        else:
            print(f"   [ERROR] 发现重复 uuid: {duplicates}")

        # 5. 确认 Device 表已删除
        print("\n5. 检查 Device 表:")
        result = conn.execute(text("""
            SELECT COUNT(*)
            FROM information_schema.tables
            WHERE table_schema = :db_name AND table_name = 'devices'
        """), {"db_name": settings.MYSQL_DATABASE})
        if result.scalar() == 0:
            print("   [OK] Device 表已删除")
        else:
            print("   [ERROR] Device 表仍然存在")

        # 6. 检查索引
        print("\n6. 检查索引:")
        result = conn.execute(text("""
            SELECT index_name, column_name, non_unique
            FROM information_schema.statistics
            WHERE table_schema = :db_name AND table_name = 'users'
            AND column_name IN ('device_code', 'uuid')
            ORDER BY index_name, seq_in_index
        """), {"db_name": settings.MYSQL_DATABASE})
        for row in result.fetchall():
            unique = "UNIQUE" if row[2] == 0 else "INDEX"
            print(f"   - {row[0]}: {row[1]} ({unique})")

        # 7. 显示示例数据
        print("\n7. 示例用户数据:")
        result = conn.execute(text("""
            SELECT id, device_code, uuid, status, password_set, activated_at
            FROM users
            LIMIT 5
        """))
        for row in result.fetchall():
            uuid_display = row[2][:16] + "..." if len(row[2]) > 16 else row[2]
            print(f"   - ID={row[0]}: code={row[1]}, uuid={uuid_display}, status={row[3]}, pwd_set={row[4]}")

        # 8. 检查 device_id 字段是否已删除
        print("\n8. 检查 device_id 字段:")
        result = conn.execute(text("""
            SELECT COUNT(*)
            FROM information_schema.columns
            WHERE table_schema = :db_name AND table_name = 'users' AND column_name = 'device_id'
        """), {"db_name": settings.MYSQL_DATABASE})
        if result.scalar() == 0:
            print("   [OK] device_id 字段已删除")
        else:
            print("   [ERROR] device_id 字段仍然存在")

    engine.dispose()
    print("\n" + "=" * 60)
    print("迁移验证完成")
    print("=" * 60)

if __name__ == "__main__":
    verify_post_migration()
