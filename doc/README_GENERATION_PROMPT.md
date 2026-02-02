# 标准化 README 生成提示词

请复制以下 Prompt 给 AI 助手，用于生成或更新项目的 README.md。

---

## Prompt 模板

```markdown
# Role
你是一位拥有丰富开源项目维护经验的技术文档专家。你擅长编写结构清晰、专业且具有吸引力的 README.md 文档。

# Context
我正在开发一个嵌入式/软件项目，需要一个符合 GitHub 标准的高质量 README 文档。
当前项目的名称是：[项目名称]
当前版本号是：[版本号，如 V1.0.0]

# Task
请通读项目文件结构、核心代码 (如 main.cpp, platformio.ini) 以及文档目录 (doc/)，生成一份标准化的 `README.md`。

# Requirements (必须遵守)

1. **头部样式 (Header)**:
   - 使用 `<div align="center">` 居中显示项目标题。
   - 标题下方必须包含 Shields.io 风格的徽章 (Badges)，至少包含：
     - License (如 MIT)
     - Platform (如 ESP32, STM32, Linux)
     - Framework (如 Arduino, Vue, React)
     - Version (从 progress.md 或代码中获取)
   - 提供一段简短精炼的项目 Slogan 或简介。
   - 提供核心链接导航 (如：文档 | 硬件定义 | 更新日志)。

2. **核心章节 (Sections)**:
   - **📖 项目简介 (Introduction)**: 简述项目背景、解决的痛点。
   - **✨ 核心特性 (Features)**: 使用列表列出功能亮点，可配合 Emoji。
   - **🛠️ 技术栈 (Tech Stack)**: 列出硬件平台、开发框架、依赖库。
   - **🔌 硬件连接 (Hardware Interface)**: (如果是硬件项目) 使用表格列出引脚映射。
   - **🚀 快速开始 (Quick Start)**: 提供环境准备、安装、编译、运行的步骤。
   - **📅 版本历史 (Changelog)**: 列出最近的几个版本及其主要变更。
   - **🤝 贡献 (Contributing)**: 鼓励开源贡献的标准话术。
   - **📄 许可证 (License)**: 声明项目的开源协议。

3. **格式规范**:
   - 语言：简体中文 (Simplified Chinese)。
   - 排版：使用清晰的 Markdown 语法，合理使用引用、代码块和表格。
   - 语气：专业、热情、客观。

4. **输入数据来源**:
   - 项目结构：[提供 tree 输出或文件列表]
   - 依赖信息：`platformio.ini` 或 `package.json`
   - 功能描述：`src/` 源码注释或 `doc/` 文档
   - 版本信息：`memory-bank/progress.md`

# Execution
请分析当前提供的项目上下文，并生成 README.md 内容。
```

---

## 使用说明

1. **直接粘贴**: 每次需要更新 README 时，将上方代码块中的内容复制并发送给 AI。
2. **补充信息**: 如果有特定的新功能需要强调，可以在发送 prompt 后补充说明，例如："这次更新重点强调了新增的 WiFi 功能"。
