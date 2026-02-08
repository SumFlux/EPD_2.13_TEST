# UI 设计规范

## 屏幕规格

- **分辨率**：212x104 像素（可见区域）
- **GRAM尺寸**：250x122 像素
- **硬件偏移**：OFFSET_Y=18（ChineseFont类自动处理）
- **字体**：匯文仿宋 16x16px（统一中英文）
- **行间距**：2px
- **行高**：18px（16px字高 + 2px间距）
- **坐标系统**：UI代码使用0-104坐标范围，ChineseFont自动加上OFFSET_Y=18

## 标准布局模板

### 模板1：单标题 + 内容

适用于：错误显示、系统就绪、简单信息展示

```
┌────────────────────────────────┐
│ 标题文本 (Y=8)                  │
│                                │
│ 内容行1 (Y=28)                 │
│ 内容行2 (Y=46)                 │
│ 内容行3 (Y=64)                 │
│ 内容行4 (Y=82)                 │
│                                │
│ 提示文本 (Y=86)                │
└────────────────────────────────┘
```

- **标题**：Y=8, 左对齐(X=10)或居中
- **内容起始**：Y=28
- **行间距**：18px（16px字高 + 2px间距）
- **最多显示**：4行内容（28 + 4×18 = 100px < 104px）
- **提示文本**：Y=86（底部提示）

### 模板2：标题 + 进度条

适用于：启动进度、下载进度、OTA更新

```
┌────────────────────────────────┐
│ 标题文本 (Y=8)                  │
│                                │
│ ┌──────────────────┐ (Y=30)   │
│ │████████░░░░░░░░░░│           │
│ └──────────────────┘           │
│                                │
│ 状态文本 (Y=56)                │
│ 百分比/详情 (Y=74)             │
└────────────────────────────────┘
```

- **标题**：Y=8
- **进度条**：Y=30, 宽180px, 高20px, 居中(X=16)
- **状态文本**：Y=56
- **详情文本**：Y=74

### 模板3：菜单列表

适用于：设置菜单、选项列表

```
┌────────────────────────────────┐
│ 标题 (Y=8)                  ↑  │
│                                │
│ > 菜单项1 (Y=28)               │
│   菜单项2 (Y=46)               │
│   菜单项3 (Y=64)               │
│   菜单项4 (Y=82)               │
│                             ↓  │
└────────────────────────────────┘
```

- **标题**：Y=8
- **菜单起始**：Y=28
- **菜单项高度**：18px（16px字高 + 2px间距）
- **最多显示**：4个菜单项（28 + 4×18 = 100px < 104px）
- **选中标记**："> " 在X=5位置
- **菜单项文本**：X=25
- **滚动支持**：当菜单项超过4个时，实现滚动机制
  - 选中项始终保持在可见区域内
  - 显示滚动指示器（↑↓箭头）
  - 滚动时使用全屏刷新（refreshFull）

### 模板4：信息展示（标签+值）

适用于：固件信息、WiFi状态、系统信息

```
┌────────────────────────────────┐
│ 标题 (Y=8)                     │
│                                │
│ 标签1: 值1 (Y=28)              │
│ 标签2: 值2 (Y=46)              │
│ 标签3: 值3 (Y=64)              │
│                                │
│ 提示文本 (Y=86)                │
└────────────────────────────────┘
```

- **标题**：Y=8
- **信息行起始**：Y=28
- **行间距**：18px
- **标签位置**：X=10
- **值位置**：X=80（标签后固定间距）
## 坐标计算公式

### 垂直布局计算

```cpp
// 行Y坐标计算
int lineY = startY + (lineIndex * 18);  // 18 = 16px字高 + 2px间距

// 示例：菜单项Y坐标
int menuStartY = 28;
int menuItemY[4] = {
  28,  // 第1项
  46,  // 第2项
  64,  // 第3项
  82   // 第4项
};
```

### 水平居中计算

```cpp
// 文本居中
int16_t textWidth = ChineseFont::getStringWidth(text);
int16_t centerX = (212 - textWidth) / 2;

// 进度条居中
int barWidth = 180;
int barX = (212 - barWidth) / 2;  // = 16
```

### 垂直居中计算

```cpp
// 单行文本垂直居中
int centerY = (104 - 16) / 2;  // = 44
```

## 安全边距

- **左右边距**：至少10px
- **顶部边距**：至少8px
- **底部边距**：至少4px
- **文本与边框间距**：2-4px

## UI组件库

### 组件1：标题文本

```cpp
/**
 * @brief 绘制标题文本（左对齐）
 * @param d EPD显示对象
 * @param text 标题文本
 */
void drawTitle(EPD_Class &d, const String &text) {
  ChineseFont::drawString(d, 10, 8, text, GxEPD_BLACK);
}
```

### 组件2：居中标题

```cpp
/**
 * @brief 绘制居中标题
 * @param d EPD显示对象
 * @param text 标题文本
 */
void drawCenteredTitle(EPD_Class &d, const String &text) {
  int16_t textWidth = ChineseFont::getStringWidth(text);
  int16_t x = (212 - textWidth) / 2;
  ChineseFont::drawString(d, x, 8, text, GxEPD_BLACK);
}
```

### 组件3：进度条

```cpp
/**
 * @brief 绘制进度条
 * @param d EPD显示对象
 * @param percentage 进度百分比 (0-100)
 */
void drawProgressBar(EPD_Class &d, int percentage) {
  int barX = 16, barY = 26, barWidth = 180, barHeight = 20;

  // 限制百分比范围
  percentage = constrain(percentage, 0, 100);
  int progress = (percentage * (barWidth - 4)) / 100;

  // 绘制边框
  d.drawRect(barX, barY, barWidth, barHeight, GxEPD_BLACK);

  // 绘制填充（内边距2px）
  if (progress > 0) {
    d.fillRect(barX + 2, barY + 2, progress, barHeight - 4, GxEPD_BLACK);
  }
}
```

### 组件4：信息行（标签+值）

```cpp
/**
 * @brief 绘制信息行（标签+值）
 * @param d EPD显示对象
 * @param y Y坐标
 * @param label 标签文本（中文）
 * @param value 值文本（可包含ASCII）
 */
void drawInfoLine(EPD_Class &d, int y, const String &label, const String &value) {
  // 绘制标签
  ChineseFont::drawString(d, 10, y, label, GxEPD_BLACK);

  // 绘制值（标签后固定间距）
  ChineseFont::drawString(d, 70, y, value, GxEPD_BLACK);
}
```

### 组件5：底部提示文本

```cpp
/**
 * @brief 绘制底部提示文本
 * @param d EPD显示对象
 * @param text 提示文本
 */
void drawHint(EPD_Class &d, const String &text) {
  ChineseFont::drawString(d, 10, 88, text, GxEPD_BLACK);
}
```

### 组件6：居中文本

```cpp
/**
 * @brief 绘制居中文本
 * @param d EPD显示对象
 * @param y Y坐标
 * @param text 文本内容
 */
void drawCenteredText(EPD_Class &d, int y, const String &text) {
  int16_t textWidth = ChineseFont::getStringWidth(text);
  int16_t x = (212 - textWidth) / 2;
  ChineseFont::drawString(d, x, y, text, GxEPD_BLACK);
}
```

### 组件7：菜单项

```cpp
/**
 * @brief 绘制菜单项
 * @param d EPD显示对象
 * @param y Y坐标
 * @param text 菜单项文本
 * @param selected 是否选中
 * @param value 可选的值文本（显示在右侧）
 */
void drawMenuItem(EPD_Class &d, int y, const String &text, bool selected, const String &value = "") {
  // 选中标记
  if (selected) {
    ChineseFont::drawString(d, 5, y, ">", GxEPD_BLACK);
  }

  // 菜单项文本
  ChineseFont::drawString(d, 20, y, text, GxEPD_BLACK);

  // 可选的值文本（右对齐）
  if (!value.isEmpty()) {
    int16_t valueWidth = ChineseFont::getStringWidth(value);
    int16_t valueX = 202 - valueWidth;  // 右边距10px
    ChineseFont::drawString(d, valueX, y, value, GxEPD_BLACK);
  }
}
```

## 完整示例代码

### 示例1：简单信息界面（系统就绪）

```cpp
// 显示"系统就绪"界面
epd.refreshFull([](EPD_Class &d) {
  d.fillScreen(GxEPD_WHITE);
  d.setTextColor(GxEPD_BLACK);

  // 标题
  ChineseFont::drawString(d, 10, 8, "系统就绪", GxEPD_BLACK);

  // WiFi状态
  if (WiFi.isConnected()) {
    ChineseFont::drawString(d, 10, 26, "WiFi:", GxEPD_BLACK);
    ChineseFont::drawString(d, 70, 26, WiFi.localIP().toString(), GxEPD_BLACK);
  } else {
    ChineseFont::drawString(d, 10, 26, "WiFi: 离线", GxEPD_BLACK);
  }

  // 卡片数量
  ChineseFont::drawString(d, 10, 40, "卡片:", GxEPD_BLACK);
  String cardCount = String(cardManager.getCardCount());
  ChineseFont::drawString(d, 70, 40, cardCount, GxEPD_BLACK);
});
```

### 示例2：进度界面（启动进度）

```cpp
// 显示启动进度界面
void showBootProgress(int step, int total, const String &message) {
  epd.refreshPartial([step, total, message](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题
    ChineseFont::drawString(d, 10, 8, "启动中", GxEPD_BLACK);

    // 进度条
    int percentage = (step * 100) / total;
    int barX = 16, barY = 26, barWidth = 180, barHeight = 20;
    int progress = (percentage * (barWidth - 4)) / 100;

    d.drawRect(barX, barY, barWidth, barHeight, GxEPD_BLACK);
    if (progress > 0) {
      d.fillRect(barX + 2, barY + 2, progress, barHeight - 4, GxEPD_BLACK);
    }

    // 步骤文本
    String stepText = "步骤 " + String(step) + "/" + String(total);
    ChineseFont::drawString(d, 10, 52, stepText, GxEPD_BLACK);

    // 状态消息
    ChineseFont::drawString(d, 10, 66, message, GxEPD_BLACK);
  });
}
```

### 示例3：菜单界面（设置菜单）

```cpp
// 显示设置菜单
void showSettingsMenu(int selectedIndex) {
  epd.refreshFull([selectedIndex](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题
    ChineseFont::drawString(d, 10, 8, "设置", GxEPD_BLACK);

    // 菜单项
    const char* menuItems[] = {
      "网络设置",
      "声音",
      "固件信息",
      "检查更新",
      "恢复出厂"
    };

    int startY = 26;
    int itemHeight = 14;

    for (int i = 0; i < 5; i++) {
      int y = startY + i * itemHeight;

      // 选中标记
      if (i == selectedIndex) {
        ChineseFont::drawString(d, 5, y, ">", GxEPD_BLACK);
      }

      // 菜单项文本
      ChineseFont::drawString(d, 20, y, menuItems[i], GxEPD_BLACK);
    }
  });
}
```

### 示例4：信息展示界面（固件信息）

```cpp
// 显示固件信息界面
void showFirmwareInfo() {
  epd.refreshFull([](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题
    ChineseFont::drawString(d, 10, 8, "固件信息", GxEPD_BLACK);

    // 版本
    ChineseFont::drawString(d, 10, 26, "版本:", GxEPD_BLACK);
    ChineseFont::drawString(d, 70, 26, FIRMWARE_VERSION, GxEPD_BLACK);

    // 构建日期
    ChineseFont::drawString(d, 10, 40, "构建:", GxEPD_BLACK);
    String buildDate = String(__DATE__) + " " + String(__TIME__);
    ChineseFont::drawString(d, 70, 40, buildDate, GxEPD_BLACK);

    // 芯片ID
    ChineseFont::drawString(d, 10, 54, "芯片:", GxEPD_BLACK);
    String chipId = String((uint32_t)ESP.getEfuseMac(), HEX);
    ChineseFont::drawString(d, 70, 54, chipId, GxEPD_BLACK);

    // 提示
    ChineseFont::drawString(d, 10, 88, "按键返回", GxEPD_BLACK);
  });
}
```

## 常见布局场景

### 场景1：错误提示

```cpp
void showError(const String &message) {
  epd.refreshFull([message](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题
    ChineseFont::drawString(d, 10, 8, "错误", GxEPD_BLACK);

    // 错误消息（支持多行，自动换行）
    ChineseFont::drawString(d, 10, 26, message, GxEPD_BLACK);
  });
}
```

### 场景2：确认对话框

```cpp
void showConfirmDialog(const String &title, const String &message) {
  epd.refreshFull([title, message](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题
    int16_t titleWidth = ChineseFont::getStringWidth(title);
    int16_t titleX = (212 - titleWidth) / 2;
    ChineseFont::drawString(d, titleX, 8, title, GxEPD_BLACK);

    // 消息（居中）
    int16_t msgWidth = ChineseFont::getStringWidth(message);
    int16_t msgX = (212 - msgWidth) / 2;
    ChineseFont::drawString(d, msgX, 40, message, GxEPD_BLACK);

    // 提示
    ChineseFont::drawString(d, 10, 88, "长按确认", GxEPD_BLACK);
  });
}
```

### 场景3：WiFi配网（二维码+文本）

```cpp
void showWiFiProvisioning(const String &ssid, const String &password) {
  epd.refreshFull([ssid, password](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题
    ChineseFont::drawString(d, 10, 8, "WiFi配置", GxEPD_BLACK);

    // 左侧：二维码（80x80）
    // renderQRCode(d, 10, 24, qrData);

    // 右侧：WiFi信息
    ChineseFont::drawString(d, 100, 28, "SSID:", GxEPD_BLACK);
    ChineseFont::drawString(d, 100, 42, ssid, GxEPD_BLACK);

    ChineseFont::drawString(d, 100, 56, "密码:", GxEPD_BLACK);
    ChineseFont::drawString(d, 100, 70, password, GxEPD_BLACK);

    ChineseFont::drawString(d, 100, 88, "扫描二维码", GxEPD_BLACK);
  });
}
```

## 设计原则

1. **一致性**：所有界面使用统一的布局模板和间距
2. **可读性**：确保文本清晰，行间距适中
3. **简洁性**：避免信息过载，每屏最多5-6行内容
4. **对齐**：文本左对齐或居中，避免右对齐
5. **层次**：使用标题、内容、提示的三层结构
6. **留白**：适当的边距和间距提升视觉效果

## 注意事项

1. **字符宽度**：所有字符（中英文）统一为12px宽
2. **行高计算**：12px字高 + 2px间距 = 14px行高
3. **最大行数**：(104 - 8 - 4) / 14 ≈ 6行（含标题）
4. **进度条尺寸**：180x20px，居中显示
5. **菜单项数量**：最多5个，确保不超出屏幕
6. **底部提示**：Y=88或Y=80，留出底部边距

## 版本历史

- **v1.0** (2026-02-08)：初始版本，定义基础布局规范
