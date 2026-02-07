# 📚 文档索引

本目录包含 InfinityTag ESP32 项目的所有技术文档。

---

## 📖 文档列表

### 🏗️ 架构设计

1. **[REFACTOR_PROGRESS.md](./REFACTOR_PROGRESS.md)** - ESP32 Lua卡片引擎架构重构进度报告
   - 总体进度：60%
   - 已完成的核心框架
   - 待实现的功能
   - 文件清单和代码统计

2. **[WIFI_PROVISIONING_REFACTOR.md](./WIFI_PROVISIONING_REFACTOR.md)** - WiFi配网架构重构报告
   - 从独立卡片改为设置中的功能
   - 新架构设计和优势
   - SettingsCard 实现细节
   - 迁移指南

---

### 🐛 问题修复

3. **[WIFI_CONFIG_FIX.md](./WIFI_CONFIG_FIX.md)** - WiFi配网问题修复报告
   - 修复WiFi启动两次的问题
   - 修复Captive Portal不工作的问题
   - Captive Portal工作原理
   - 测试步骤和注意事项

4. **[CARD_INDEX_FIX.md](./CARD_INDEX_FIX.md)** - 卡片索引问题修复报告
   - 修复无效的卡片索引 -1
   - 在正常模式下设置默认卡片
   - 添加防御性检查
   - 图标生成工具

5. **[CARD_SWITCH_OPTIMIZATION.md](./CARD_SWITCH_OPTIMIZATION.md)** - 卡片切换逻辑优化报告
   - 避免重复进入 Config Mode
   - 智能卡片切换（同卡片检测）
   - 取消切换优化
   - 性能提升和用户体验改进

---

### 🔧 配置指南

6. **[NETWORK_SETUP.md](./NETWORK_SETUP.md)** - 网络配置指南
   - WiFi连接配置
   - 网络调试方法
   - 常见问题解决

7. **[DEBUG_GUIDE.md](./DEBUG_GUIDE.md)** - 调试指南
   - 串口调试方法
   - 常见错误排查
   - 日志分析技巧

---

## 🗂️ 文档分类

### 按主题分类

#### 架构与设计
- REFACTOR_PROGRESS.md
- WIFI_PROVISIONING_REFACTOR.md

#### 问题修复
- WIFI_CONFIG_FIX.md
- CARD_INDEX_FIX.md
- CARD_SWITCH_OPTIMIZATION.md

#### 配置与调试
- NETWORK_SETUP.md
- DEBUG_GUIDE.md

---

### 按阅读顺序推荐

#### 新手入门
1. README.md（项目根目录）
2. NETWORK_SETUP.md
3. DEBUG_GUIDE.md

#### 开发者
1. REFACTOR_PROGRESS.md（了解整体架构）
2. WIFI_PROVISIONING_REFACTOR.md（了解配网设计）
3. 各个修复文档（了解问题和解决方案）

#### 问题排查
1. DEBUG_GUIDE.md
2. 对应的修复文档（根据具体问题）

---

## 📊 文档统计

- **总文档数**：7个
- **架构设计**：2个
- **问题修复**：3个
- **配置指南**：2个
- **总字数**：约 50,000 字

---

## 🔄 文档更新记录

| 文档 | 最后更新 | 版本 |
|------|----------|------|
| REFACTOR_PROGRESS.md | 2026-02-07 | v1.0 |
| WIFI_PROVISIONING_REFACTOR.md | 2026-02-07 | v2.0 |
| WIFI_CONFIG_FIX.md | 2026-02-07 | v1.0 |
| CARD_INDEX_FIX.md | 2026-02-07 | v1.1 |
| CARD_SWITCH_OPTIMIZATION.md | 2026-02-07 | v1.2 |
| NETWORK_SETUP.md | 2026-02-07 | - |
| DEBUG_GUIDE.md | 2026-02-07 | - |

---

## 📝 文档编写规范

所有技术文档遵循以下规范：

1. **使用 Markdown 格式**
2. **包含清晰的标题层级**
3. **提供代码示例和日志输出**
4. **包含测试步骤和预期结果**
5. **标注版本号和更新时间**

---

## 🤝 贡献指南

如果你想添加或更新文档：

1. 在 `/docs` 目录下创建或修改文档
2. 使用清晰的文件名（英文，下划线分隔）
3. 更新本索引文件（README.md）
4. 提交 Pull Request

---

## 📧 联系方式

如有问题或建议，请通过以下方式联系：

- GitHub Issues
- 项目讨论区

---

最后更新：2026-02-07
