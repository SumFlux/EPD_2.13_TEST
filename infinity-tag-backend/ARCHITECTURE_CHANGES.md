# 架构变更说明 - Device 表合并到 User 表

**日期**: 2026-02-15
**版本**: v2.0
**迁移版本**: e44613d36e3a

---

## 📋 变更概述

将 Device 表合并到 User 表，实现"设备即用户"的简化架构。

### 变更前
```
User 表 ←→ Device 表 (1:1 外键关系)
├─ device_id (FK → Device.device_code)
└─ password_hash

Device 表
├─ device_code (6位码)
├─ uuid
├─ status
└─ init_password_hash
```

### 变更后
```
User 表 (设备即用户)
├─ device_code (设备码，10字符)
├─ uuid (ESP32 UUID)
├─ status (pending/activated/disabled)
├─ init_password_hash (初始密码)
└─ password_hash (用户密码)
```

---

## 🔄 数据库迁移

### 执行迁移

```bash
# 备份数据库（重要！）
mysqldump -u root -p infinity_tag > backup_$(date +%Y%m%d).sql

# 执行迁移
alembic upgrade head

# 验证迁移
python verify_migration.py
```

### 迁移内容

1. ✅ 添加新字段到 User 表（device_code, uuid, status, init_password_hash）
2. ✅ 迁移 Device 数据到 User 表
3. ✅ 处理孤立设备（创建新 User）
4. ✅ 处理无效 device_id（生成虚拟设备数据）
5. ✅ 创建唯一约束和索引
6. ✅ 删除 device_id 字段和 Device 表

### 回滚（不推荐）

```bash
# 注意：回滚功能存在问题，不建议使用
# 如需回滚，请恢复数据库备份
mysql -u root -p infinity_tag < backup_YYYYMMDD.sql
```

---

## 💻 代码变更

### 后端变更

**删除的文件**:
- `app/models/device.py`
- `app/repositories/device_repository.py`
- `app/services/device_service.py`

**修改的文件**:
- `app/models/user.py` - 添加设备字段
- `app/repositories/user_repo.py` - 新增查询方法
- `app/services/auth_service.py` - 直接使用 User 模型
- `app/services/admin_service.py` - 设备管理改为用户管理
- `app/api/v1/endpoints/admin.py` - 更新端点实现
- `app/api/v1/endpoints/device.py` - 简化为查询 User
- `app/schemas/admin.py` - 更新类型定义

### 前端变更

**修改的文件**:
- `src/types/admin.ts` - 更新类型定义
- `src/api/admin.ts` - 更新 API 调用

---

## 🔌 API 变更

### 管理后台 API

**保持不变的端点**:
- `POST /api/v1/admin/devices` - 创建设备（内部创建 User）
- `GET /api/v1/admin/devices` - 设备列表（查询 User 表）
- `POST /api/v1/admin/devices/batch` - 批量导入
- `GET /api/v1/admin/devices/{device_code}` - 设备详情
- `PUT /api/v1/admin/devices/{device_code}/disable` - 禁用设备

**参数变更**:
- 设备详情和禁用接口：`device_id` (int) → `device_code` (string)

**删除的端点**:
- `PUT /api/v1/admin/devices/{device_id}/reset` - 重置设备
- `DELETE /api/v1/admin/devices/{device_id}` - 删除设备

### 认证 API

**无变更** - 所有认证端点保持不变

---

## 📊 数据库架构

### User 表结构

```sql
CREATE TABLE users (
    id INT PRIMARY KEY AUTO_INCREMENT,

    -- 设备字段（新增）
    device_code VARCHAR(10) NOT NULL UNIQUE,
    init_password_hash VARCHAR(255) NOT NULL,
    uuid VARCHAR(64) NOT NULL UNIQUE,
    status VARCHAR(20) NOT NULL DEFAULT 'pending',

    -- 用户字段
    password_hash VARCHAR(255),
    device_secret VARCHAR(64) NOT NULL,
    password_set BOOLEAN NOT NULL DEFAULT FALSE,
    activated_at DATETIME,
    last_login_at DATETIME,

    created_at DATETIME NOT NULL,
    updated_at DATETIME NOT NULL,

    INDEX ix_users_device_code (device_code),
    INDEX ix_users_uuid (uuid)
);
```

### 删除的表

- ❌ `devices` 表已删除

---

## ⚠️ 注意事项

### 1. 字段长度变更

device_code 从 6 字符扩展到 10 字符，以兼容历史数据。

### 2. 虚拟设备数据

迁移过程中，3 个用户的 device_id 指向不存在的设备，系统自动生成了虚拟设备数据：
- User ID=1: device_code=YTQGKZ
- User ID=2: device_code=sumhello
- User ID=3: device_code=LKY25K

**建议**: 检查这些用户是否为测试数据，如是则删除。

### 3. 回滚限制

回滚功能存在字段长度不匹配问题，不建议使用。如需回滚，请恢复数据库备份。

---

## ✅ 验证清单

迁移后请验证以下功能：

- [ ] 应用启动正常
- [ ] 设备激活流程正常
- [ ] 用户登录流程正常
- [ ] 管理后台设备列表显示正常
- [ ] 管理后台设备创建正常
- [ ] 管理后台批量导入正常
- [ ] 统计数据正确

---

## 📚 相关文档

- [完整迁移报告](MIGRATION_REPORT.md)
- [实施总结](../../../openspec/changes/device-user-merge/IMPLEMENTATION_SUMMARY.md)
- [PRD 实施差异分析](../../../docs/reports/2026-02-14-prd-implementation-gap-analysis.md)

---

**更新日期**: 2026-02-15
**迁移状态**: ✅ 已完成
