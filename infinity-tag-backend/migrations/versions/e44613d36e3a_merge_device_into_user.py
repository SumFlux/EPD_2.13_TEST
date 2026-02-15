"""merge_device_into_user

Revision ID: e44613d36e3a
Revises: 5a5ded598b4b
Create Date: 2026-02-15 02:39:10.327409

"""
from typing import Sequence, Union

from alembic import op
import sqlalchemy as sa
from sqlalchemy import text


# revision identifiers, used by Alembic.
revision: str = 'e44613d36e3a'
down_revision: Union[str, Sequence[str], None] = '5a5ded598b4b'
branch_labels: Union[str, Sequence[str], None] = None
depends_on: Union[str, Sequence[str], None] = None


def upgrade() -> None:
    """Merge Device table into User table."""
    conn = op.get_bind()

    # Pre-migration validation
    print("[Migration] Starting pre-migration validation...")
    device_count = conn.execute(text("SELECT COUNT(*) FROM devices")).scalar()
    user_count = conn.execute(text("SELECT COUNT(*) FROM users")).scalar()
    print(f"[Migration] Found {device_count} devices and {user_count} users")

    # Step 1: Add new columns to users table (nullable first)
    print("[Migration] Step 1: Adding new columns to users table...")
    op.add_column('users', sa.Column('device_code', sa.String(10), nullable=True))
    op.add_column('users', sa.Column('init_password_hash', sa.String(255), nullable=True))
    op.add_column('users', sa.Column('uuid', sa.String(64), nullable=True))
    op.add_column('users', sa.Column('status', sa.String(20), nullable=True))

    # Step 2: Migrate data from devices to users (for users with device_id)
    print("[Migration] Step 2: Migrating device data to existing users...")
    conn.execute(text("""
        UPDATE users u
        INNER JOIN devices d ON u.device_id = d.device_code
        SET
            u.device_code = d.device_code,
            u.init_password_hash = d.init_password_hash,
            u.uuid = d.uuid,
            u.status = d.status,
            u.activated_at = COALESCE(u.activated_at, d.activated_at)
    """))
    migrated_count = conn.execute(text(
        "SELECT COUNT(*) FROM users WHERE device_code IS NOT NULL"
    )).scalar()
    print(f"[Migration] Migrated {migrated_count} users with device data")

    # Step 2b: Handle users with invalid device_id (device doesn't exist)
    print("[Migration] Step 2b: Handling users with invalid device_id...")
    orphaned_users = conn.execute(text("""
        SELECT u.id, u.device_id, u.device_secret
        FROM users u
        LEFT JOIN devices d ON u.device_id = d.device_code
        WHERE d.device_code IS NULL
    """)).fetchall()

    if orphaned_users:
        print(f"[Migration] Found {len(orphaned_users)} users with invalid device_id")
        import secrets
        from passlib.context import CryptContext
        pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")

        for user in orphaned_users:
            user_id, device_code, device_secret = user
            # Generate a random UUID and initial password for these users
            fake_uuid = secrets.token_hex(16)
            init_password = secrets.token_urlsafe(12)
            init_password_hash = pwd_context.hash(init_password)

            print(f"[Migration]   User ID={user_id}, device_code={device_code}, generated uuid={fake_uuid[:16]}...")

            conn.execute(text("""
                UPDATE users
                SET device_code = :device_code,
                    init_password_hash = :init_password_hash,
                    uuid = :uuid,
                    status = 'activated'
                WHERE id = :user_id
            """), {
                'user_id': user_id,
                'device_code': device_code,
                'init_password_hash': init_password_hash,
                'uuid': fake_uuid,
            })
    else:
        print("[Migration] No users with invalid device_id found")

    # Step 3: Make device_id nullable temporarily (for orphaned device insertion)
    print("[Migration] Step 3a: Making device_id nullable temporarily...")
    op.alter_column('users', 'device_id',
                    existing_type=sa.String(10),
                    nullable=True)

    # Step 3b: Handle orphaned devices (devices without associated users)
    print("[Migration] Step 3b: Handling orphaned devices...")
    orphaned_devices = conn.execute(text("""
        SELECT d.device_code, d.init_password_hash, d.uuid, d.status, d.activated_at
        FROM devices d
        LEFT JOIN users u ON u.device_id = d.device_code
        WHERE u.id IS NULL
    """)).fetchall()

    orphaned_count = len(orphaned_devices)
    if orphaned_count > 0:
        print(f"[Migration] Found {orphaned_count} orphaned devices, creating new users...")
        for device in orphaned_devices:
            # Generate device_secret for orphaned devices
            import secrets
            device_secret = secrets.token_hex(32)

            conn.execute(text("""
                INSERT INTO users (
                    device_id, device_code, init_password_hash, uuid, status,
                    device_secret, password_set, activated_at,
                    created_at, updated_at
                )
                VALUES (
                    :device_code, :device_code, :init_password_hash, :uuid, :status,
                    :device_secret, FALSE, :activated_at,
                    NOW(), NOW()
                )
            """), {
                'device_code': device[0],
                'init_password_hash': device[1],
                'uuid': device[2],
                'status': device[3],
                'device_secret': device_secret,
                'activated_at': device[4]
            })
    else:
        print("[Migration] No orphaned devices found")

    # Step 4: Set columns to NOT NULL and add constraints
    print("[Migration] Step 4: Setting NOT NULL constraints and creating indexes...")
    op.alter_column('users', 'device_code',
                    existing_type=sa.String(10),
                    nullable=False)
    op.alter_column('users', 'init_password_hash',
                    existing_type=sa.String(255),
                    nullable=False)
    op.alter_column('users', 'uuid',
                    existing_type=sa.String(64),
                    nullable=False)
    op.alter_column('users', 'status',
                    existing_type=sa.String(20),
                    nullable=False,
                    server_default='pending')

    # Create unique constraints and indexes
    op.create_unique_constraint('uq_users_device_code', 'users', ['device_code'])
    op.create_unique_constraint('uq_users_uuid', 'users', ['uuid'])
    op.create_index('ix_users_uuid', 'users', ['uuid'])

    # Step 5: Drop old device_id column and foreign key
    print("[Migration] Step 5: Dropping device_id foreign key and column...")
    # Drop foreign key constraint if it exists
    try:
        op.drop_constraint('users_device_id_fkey', 'users', type_='foreignkey')
    except:
        print("[Migration] Foreign key constraint not found, skipping...")

    op.drop_column('users', 'device_id')

    # Step 6: Drop devices table
    print("[Migration] Step 6: Dropping devices table...")
    op.drop_table('devices')

    # Post-migration validation
    print("[Migration] Post-migration validation...")
    final_user_count = conn.execute(text("SELECT COUNT(*) FROM users")).scalar()
    expected_count = user_count + orphaned_count
    print(f"[Migration] Final user count: {final_user_count} (expected: {expected_count})")

    if final_user_count != expected_count:
        raise Exception(f"Migration validation failed: expected {expected_count} users, got {final_user_count}")

    print("[Migration] Migration completed successfully!")


def downgrade() -> None:
    """Rollback: Recreate Device table and restore data."""
    conn = op.get_bind()

    print("[Rollback] Starting rollback process...")

    # Step 1: Recreate devices table
    print("[Rollback] Step 1: Recreating devices table...")
    op.create_table(
        'devices',
        sa.Column('id', sa.Integer(), nullable=False),
        sa.Column('device_code', sa.String(6), nullable=False),
        sa.Column('init_password_hash', sa.String(255), nullable=False),
        sa.Column('uuid', sa.String(64), nullable=False),
        sa.Column('status', sa.String(20), nullable=False),
        sa.Column('user_id', sa.Integer(), nullable=True),
        sa.Column('batch_name', sa.String(100), nullable=True),
        sa.Column('activated_at', sa.DateTime(), nullable=True),
        sa.Column('notes', sa.Text(), nullable=True),
        sa.Column('created_at', sa.DateTime(), nullable=False),
        sa.Column('updated_at', sa.DateTime(), nullable=False),
        sa.PrimaryKeyConstraint('id'),
        sa.UniqueConstraint('device_code'),
        sa.UniqueConstraint('uuid')
    )
    op.create_index('ix_devices_device_code', 'devices', ['device_code'])

    # Step 2: Restore device data from users table
    print("[Rollback] Step 2: Restoring device data...")
    conn.execute(text("""
        INSERT INTO devices (
            device_code, init_password_hash, uuid, status,
            user_id, activated_at, created_at, updated_at
        )
        SELECT
            device_code, init_password_hash, uuid, status,
            id as user_id, activated_at, created_at, updated_at
        FROM users
    """))

    # Step 3: Add device_id column back to users
    print("[Rollback] Step 3: Adding device_id column back to users...")
    op.add_column('users', sa.Column('device_id', sa.String(10), nullable=True))

    # Step 4: Populate device_id with device_code
    conn.execute(text("""
        UPDATE users
        SET device_id = device_code
    """))

    # Step 5: Set device_id to NOT NULL and create constraints
    op.alter_column('users', 'device_id', nullable=False)
    op.create_unique_constraint('uq_users_device_id', 'users', ['device_id'])
    op.create_index('ix_users_device_id', 'users', ['device_id'])

    # Step 6: Create foreign key relationship
    print("[Rollback] Step 4: Creating foreign key relationship...")
    op.create_foreign_key(
        'users_device_id_fkey',
        'users', 'devices',
        ['device_id'], ['device_code'],
        ondelete='SET NULL'
    )

    # Step 7: Drop new columns from users table
    print("[Rollback] Step 5: Dropping merged columns from users...")
    op.drop_index('ix_users_uuid', 'users')
    op.drop_constraint('uq_users_uuid', 'users', type_='unique')
    op.drop_constraint('uq_users_device_code', 'users', type_='unique')

    op.drop_column('users', 'status')
    op.drop_column('users', 'uuid')
    op.drop_column('users', 'init_password_hash')
    op.drop_column('users', 'device_code')

    print("[Rollback] ✓ Rollback completed successfully!")
