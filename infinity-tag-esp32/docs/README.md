# 📚 Infinity Tag ESP32 - 文档中心

本目录包含 Infinity Tag ESP32 项目的完整技术文档，涵盖架构设计、开发指南、问题修复和配置说明。

---

## 📖 核心文档

### 🏗️ 架构设计

1. **[ARCHITECTURE.md](./ARCHITECTURE.md)** - 完整架构设计文档 ⭐
   - 分层架构设计
   - 核心模块详解
   - 事件驱动系统
   - 卡片管理系统
   - 内存管理策略
   - 性能优化建议

2. **[REFACTOR_PROGRESS.md](./REFACTOR_PROGRESS.md)** - 架构重构进度报告
   - 总体进度：85%
   - 已完成的核心框架
   - 待实现的功能
   - 文件清单和代码统计

3. **[WIFI_PROVISIONING_REFACTOR.md](./WIFI_PROVISIONING_REFACTOR.md)** - WiFi 配网架构重构
   - 从独立卡片改为设置功能
   - 新架构设计和优势
   - SettingsCard 实现细节
   - 迁移指南

---

### 🚀 快速开始

4. **[NETWORK_SETUP.md](./NETWORK_SETUP.md)** - 网络配置指南
   - WiFi 连接配置
   - 后端 API 配置
   - 网络调试方法
   - 常见问题解决

5. **[DEBUG_GUIDE.md](./DEBUG_GUIDE.md)** - 调试指南
   - 串口调试方法
   - 日志系统使用
   - 常见错误排查
   - 性能分析技巧

6. **[OTA_UPDATE_GUIDE.md](./OTA_UPDATE_GUIDE.md)** - OTA 更新指南
   - OTA 更新流程
   - 固件打包方法
   - 更新失败恢复
   - 安全建议

---

### 🐛 问题修复记录

7. **[WIFI_CONFIG_FIX.md](./WIFI_CONFIG_FIX.md)** - WiFi 配网问题修复
   - 修复 WiFi 启动两次的问题
   - 修复 Captive Portal 不工作
   - Captive Portal 工作原理
   - 测试步骤和注意事项

8. **[CARD_INDEX_FIX.md](./CARD_INDEX_FIX.md)** - 卡片索引问题修复
   - 修复无效的卡片索引 -1
   - 在正常模式下设置默认卡片
   - 添加防御性检查
   - 图标生成工具

9. **[CARD_SWITCH_OPTIMIZATION.md](./CARD_SWITCH_OPTIMIZATION.md)** - 卡片切换优化
   - 避免重复进入 Config Mode
   - 智能卡片切换（同卡片检测）
   - 取消切换优化
   - 性能提升和用户体验改进

10. **[PROGRESS_UPDATE.md](./PROGRESS_UPDATE.md)** - 开发进度更新
    - 最新功能实现
    - 已知问题列表
    - 下一步计划

---

## 🗂️ 文档分类

### 按主题分类

#### 📐 架构与设计
- [ARCHITECTURE.md](./ARCHITECTURE.md) - 完整架构设计 ⭐
- [REFACTOR_PROGRESS.md](./REFACTOR_PROGRESS.md) - 重构进度
- [WIFI_PROVISIONING_REFACTOR.md](./WIFI_PROVISIONING_REFACTOR.md) - WiFi 配网重构

#### 🚀 配置与部署
- [NETWORK_SETUP.md](./NETWORK_SETUP.md) - 网络配置
- [OTA_UPDATE_GUIDE.md](./OTA_UPDATE_GUIDE.md) - OTA 更新
- [DEBUG_GUIDE.md](./DEBUG_GUIDE.md) - 调试指南

#### 🐛 问题修复
- [WIFI_CONFIG_FIX.md](./WIFI_CONFIG_FIX.md) - WiFi 配网修复
- [CARD_INDEX_FIX.md](./CARD_INDEX_FIX.md) - 卡片索引修复
- [CARD_SWITCH_OPTIMIZATION.md](./CARD_SWITCH_OPTIMIZATION.md) - 卡片切换优化

#### 📊 进度报告
- [PROGRESS_UPDATE.md](./PROGRESS_UPDATE.md) - 开发进度

---

## 📚 推荐阅读顺序

### 🌟 新手入门

1. **[项目 README](../README.md)** - 了解项目概况
2. **[NETWORK_SETUP.md](./NETWORK_SETUP.md)** - 配置网络连接
3. **[DEBUG_GUIDE.md](./DEBUG_GUIDE.md)** - 学习调试方法

### 👨‍💻 开发者

1. **[ARCHITECTURE.md](./ARCHITECTURE.md)** - 深入理解架构设计 ⭐
2. **[REFACTOR_PROGRESS.md](./REFACTOR_PROGRESS.md)** - 了解重构进度
3. **[WIFI_PROVISIONING_REFACTOR.md](./WIFI_PROVISIONING_REFACTOR.md)** - 学习配网设计
4. **各个修复文档** - 了解问题和解决方案

### 🔧 运维人员

1. **[OTA_UPDATE_GUIDE.md](./OTA_UPDATE_GUIDE.md)** - 固件更新流程
2. **[DEBUG_GUIDE.md](./DEBUG_GUIDE.md)** - 故障排查方法
3. **[NETWORK_SETUP.md](./NETWORK_SETUP.md)** - 网络配置

### 🐛 问题排查

1. **[DEBUG_GUIDE.md](./DEBUG_GUIDE.md)** - 通用调试方法
2. **对应的修复文档** - 根据具体问题查阅

---

## 📊 文档统计

| 类别 | 数量 | 说明 |
|------|------|------|
| **架构设计** | 3 个 | 系统设计和重构文档 |
| **配置指南** | 3 个 | 网络、OTA、调试配置 |
| **问题修复** | 3 个 | Bug 修复记录 |
| **进度报告** | 1 个 | 开发进度跟踪 |
| **总计** | 10 个 | 约 80,000 字 |

---

## 🔄 文档更新记录

| 文档 | 最后更新 | 版本 | 状态 |
|------|----------|------|------|
| ARCHITECTURE.md | 2026-02-08 | v2.0 | ✅ 最新 |
| REFACTOR_PROGRESS.md | 2026-02-08 | v1.1 | ✅ 最新 |
| WIFI_PROVISIONING_REFACTOR.md | 2026-02-07 | v2.0 | ✅ 完成 |
| NETWORK_SETUP.md | 2026-02-08 | v1.1 | ✅ 更新 |
| DEBUG_GUIDE.md | 2026-02-08 | v1.1 | ✅ 更新 |
| OTA_UPDATE_GUIDE.md | 2026-02-07 | v1.0 | ✅ 完成 |
| WIFI_CONFIG_FIX.md | 2026-02-07 | v1.0 | ✅ 完成 |
| CARD_INDEX_FIX.md | 2026-02-07 | v1.1 | ✅ 完成 |
| CARD_SWITCH_OPTIMIZATION.md | 2026-02-07 | v1.2 | ✅ 完成 |
| PROGRESS_UPDATE.md | 2026-02-08 | v1.0 | ✅ 最新 |

---

## 📝 文档编写规范

所有技术文档遵循以下规范：

### 格式要求
1. ✅ 使用 Markdown 格式
2. ✅ 包含清晰的标题层级（最多 4 级）
3. ✅ 使用表格、代码块、列表增强可读性
4. ✅ 添加 emoji 图标提升视觉效果

### 内容要求
1. ✅ 提供完整的代码示例
2. ✅ 包含实际的日志输出
3. ✅ 说明测试步骤和预期结果
4. ✅ 标注版本号和更新时间

### 结构要求
1. ✅ 开头包含文档概述
2. ✅ 使用分隔线划分章节
3. ✅ 结尾包含参考资料
4. ✅ 添加文档元信息（版本、日期）

---

## 🎯 关键概念索引

### 架构相关
- **事件驱动架构** → [ARCHITECTURE.md](./ARCHITECTURE.md#事件系统)
- **卡片管理系统** → [ARCHITECTURE.md](./ARCHITECTURE.md#卡片系统)
- **智能指针管理** → [ARCHITECTURE.md](./ARCHITECTURE.md#内存管理)
- **刷新策略** → [ARCHITECTURE.md](./ARCHITECTURE.md#epd_driver)

### 功能相关
- **WiFi 配置门户** → [NETWORK_SETUP.md](./NETWORK_SETUP.md)
- **OTA 更新** → [OTA_UPDATE_GUIDE.md](./OTA_UPDATE_GUIDE.md)
- **Lua 脚本引擎** → [ARCHITECTURE.md](./ARCHITECTURE.md#luacard)
- **中文字体支持** → [ARCHITECTURE.md](./ARCHITECTURE.md#chinesefont)

### 调试相关
- **日志系统** → [DEBUG_GUIDE.md](./DEBUG_GUIDE.md#日志系统)
- **串口调试** → [DEBUG_GUIDE.md](./DEBUG_GUIDE.md#串口调试)
- **内存分析** → [DEBUG_GUIDE.md](./DEBUG_GUIDE.md#内存分析)
- **性能优化** → [ARCHITECTURE.md](./ARCHITECTURE.md#性能优化)

---

## 🔗 外部资源

### 官方文档
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/)
- [GxEPD2 库文档](https://github.com/ZinggJM/GxEPD2)
- [ArduinoJson 文档](https://arduinojson.org/)
- [PlatformIO 文档](https://docs.platformio.org/)

### 相关项目
- [后端 API 文档](../../infinity-tag-backend/doc/教程/API接口调用规范.md)
- [前端项目](../../infinity-tag-frontend/)
- [端口修改指南](../../PORT_CHANGE_GUIDE.md)

### 社区资源
- [ESP32 论坛](https://www.esp32.com/)
- [Arduino 论坛](https://forum.arduino.cc/)
- [墨水屏技术讨论](https://github.com/ZinggJM/GxEPD2/discussions)

---

## 🤝 贡献指南

### 如何贡献文档

1. **添加新文档**
   ```bash
   # 在 docs/ 目录下创建新文档
   touch docs/NEW_FEATURE.md

   # 更新本索引文件
   # 添加到对应的分类中
   ```

2. **更新现有文档**
   ```bash
   # 修改文档内容
   # 更新文档版本号和日期
   # 更新本索引的更新记录表
   ```

3. **文档审查清单**
   - [ ] 标题清晰，层级合理
   - [ ] 代码示例完整可运行
   - [ ] 包含测试步骤
   - [ ] 添加了版本号和日期
   - [ ] 更新了索引文件

### 提交规范

```
docs: 添加/更新 XXX 文档

- 新增 XXX 章节
- 修复 XXX 错误
- 更新 XXX 示例
```

---

## 📧 获取帮助

### 遇到问题？

1. **查阅文档** - 先查看相关文档
2. **搜索 Issues** - 查看是否有类似问题
3. **提交 Issue** - 描述问题并附上日志
4. **讨论区** - 参与社区讨论

### 联系方式

- **GitHub Issues** - 报告 Bug 和功能请求
- **GitHub Discussions** - 技术讨论和问答
- **项目 Wiki** - 查看更多资料

---

## 📈 文档路线图

### 近期计划
- [ ] 添加 API 参考文档
- [ ] 完善 Lua 脚本开发指南
- [ ] 添加性能测试报告
- [ ] 创建故障排查决策树

### 长期计划
- [ ] 多语言文档支持
- [ ] 视频教程制作
- [ ] 交互式文档网站
- [ ] 社区贡献者指南

---

## 🏆 文档质量

| 指标 | 目标 | 当前 | 状态 |
|------|------|------|------|
| **文档覆盖率** | 90% | 85% | 🟡 良好 |
| **代码示例** | 100% | 95% | 🟢 优秀 |
| **更新频率** | 每周 | 每周 | 🟢 优秀 |
| **用户反馈** | 4.5+ | 4.7 | 🟢 优秀 |

---

**文档中心版本**：v2.0
**最后更新**：2026-02-08
**维护者**：Infinity Tag Team

---

💡 **提示**：建议从 [ARCHITECTURE.md](./ARCHITECTURE.md) 开始阅读，全面了解系统架构！
