#include "Core/CardManager.h"
#include "Utils/ChineseFont.h"
#include <LittleFS.h>

CardManager::CardManager(EPD_Driver &epd, StatusBar &statusBar)
    : _epd(epd), _statusBar(statusBar), _currentCardIndex(-1),
      _state(STATE_NORMAL), _longPressStartTime(0), _isLongPressing(false),
      _switchPreviewIndex(0), _switchModeStartTime(0) {}

CardManager::~CardManager() {
  // 注意：CardManager不负责释放Card对象的内存
  // Card对象应该由创建者管理
}

bool CardManager::begin() {
  Serial.println("[CardManager] Initialized");
  return true;
}

void CardManager::registerCard(Card *card) {
  if (card == nullptr) {
    Serial.println("[CardManager] ERROR: Cannot register null card");
    return;
  }

  _cards.push_back(card);
  Serial.printf("[CardManager] Registered card: %s (%s)\n",
                card->getName().c_str(), card->getCategory().c_str());

  // 注意：不自动设置当前卡片，由调用者决定何时激活
}

bool CardManager::setCurrentCard(int index) {
  if (index < 0 || index >= static_cast<int>(_cards.size())) {
    Serial.printf("[CardManager] ERROR: Invalid card index: %d\n", index);
    return false;
  }

  // 如果切换回同一张卡片，不触发完整的 onExit/onEnter，避免重复初始化
  if (_currentCardIndex == index) {
    Serial.printf(
        "[CardManager] Same card selected (index: %d), skipping lifecycle\n",
        index);
    return true;
  }

  // 退出当前卡片
  if (_currentCardIndex >= 0 &&
      _currentCardIndex < static_cast<int>(_cards.size())) {
    _cards[_currentCardIndex]->onExit();
  }

  // 切换到新卡片
  _currentCardIndex = index;
  _cards[_currentCardIndex]->onEnter();

  Serial.printf("[CardManager] Switched to card: %s\n",
                _cards[_currentCardIndex]->getName().c_str());

  return true;
}

void CardManager::processEvents(EventQueue &eventQueue) {
  // 检查切换模式超时
  if (_state == STATE_SWITCHING) {
    _checkSwitchModeTimeout();
  }

  // 处理事件队列
  Event event;
  while (eventQueue.pop(event)) {
    _handleEvent(event);
  }
}

void CardManager::_handleEvent(const Event &event) {
  switch (_state) {
  case STATE_NORMAL:
    _handleNormalEvent(event);
    break;

  case STATE_SWITCHING:
    _handleSwitchingEvent(event);
    break;

  case STATE_TRANSITIONING:
    // 过渡动画期间不处理事件
    break;
  }
}

void CardManager::_handleNormalEvent(const Event &event) {
  // 检查是否有有效的当前卡片
  if (_currentCardIndex < 0 ||
      _currentCardIndex >= static_cast<int>(_cards.size())) {
    Serial.printf("[CardManager] WARNING: No valid current card (index: %d), "
                  "ignoring event\n",
                  _currentCardIndex);
    return;
  }

  switch (event.type) {
  case EVENT_BUTTON_PRESS:
    // 检测长按开始
    _isLongPressing = true;
    _longPressStartTime = millis();
    break;

  case EVENT_BUTTON_RELEASE:
    // 检测长按结束
    if (_isLongPressing) {
      uint32_t pressDuration = millis() - _longPressStartTime;
      if (pressDuration >= LONG_PRESS_DURATION) {
        // 长按1秒，进入切换模式
        _enterSwitchMode();
      } else {
        // 短按，传递给当前卡片
        if (_currentCardIndex >= 0) {
          _cards[_currentCardIndex]->onEvent(event);
        }
      }
      _isLongPressing = false;
    }
    break;

  case EVENT_BUTTON_LONG_PRESS:
    // 长按事件（由InputManager产生）
    _enterSwitchMode();
    _isLongPressing = false;
    break;

  case EVENT_BUTTON_TRIPLE_CLICK:
    // 三击事件，触发OTA检查（由其他组件处理）
    // 这里只是记录日志
    Serial.println("[CardManager] Triple click detected");
    break;

  default:
    // 其他事件传递给当前卡片
    if (_currentCardIndex >= 0) {
      _cards[_currentCardIndex]->onEvent(event);
    }
    break;
  }
}

void CardManager::_handleSwitchingEvent(const Event &event) {
  switch (event.type) {
  case EVENT_ENCODER_ROTATE:
    // 左右滚动切换卡片预览
    _switchPreviewIndex += event.value;

    // 循环索引
    if (_switchPreviewIndex < 0) {
      _switchPreviewIndex = _cards.size() - 1;
    } else if (_switchPreviewIndex >= static_cast<int>(_cards.size())) {
      _switchPreviewIndex = 0;
    }

    // 重新渲染切换界面
    _renderCardSwitchUI();
    break;

  case EVENT_BUTTON_RELEASE:
    // 松开滚轮，切换到选中的卡片
    _exitSwitchMode();
    break;

  default:
    // 其他事件忽略
    break;
  }
}

void CardManager::_enterSwitchMode() {
  Serial.println("[CardManager] Entering switch mode");

  _state = STATE_SWITCHING;
  _switchPreviewIndex = _currentCardIndex;
  _switchModeStartTime = millis();

  // 全屏闪白
  _flashWhite();

  // 显示卡片选择界面
  _renderCardSwitchUI();
}

void CardManager::_exitSwitchMode() {
  Serial.println("[CardManager] Exiting switch mode");

  _state = STATE_TRANSITIONING;

  // 全屏闪白
  _flashWhite();

  // 显示过渡动画（"分类/卡片名"）
  _renderTransition();

  // 切换到选中的卡片
  setCurrentCard(_switchPreviewIndex);

  // 返回正常模式
  _state = STATE_NORMAL;
}

void CardManager::_cancelSwitchMode() {
  Serial.println("[CardManager] Canceling switch mode");

  _state = STATE_NORMAL;

  // 全屏闪白
  _flashWhite();

  // 不需要重新进入当前卡片，因为我们没有离开过
  // 只需要重新渲染当前卡片即可
  Serial.printf("[CardManager] Staying on card: %s\n",
                _cards[_currentCardIndex]->getName().c_str());
}

void CardManager::_checkSwitchModeTimeout() {
  if (millis() - _switchModeStartTime > SWITCH_MODE_TIMEOUT) {
    Serial.println("[CardManager] Switch mode timeout");
    _cancelSwitchMode();
  }
}

void CardManager::_renderCardSwitchUI() {
  Serial.println("[CardManager] Rendering card switch UI");

  // 捕获需要的变量
  int previewIdx = _switchPreviewIndex;
  int cardsSize = static_cast<int>(_cards.size());

  _epd.refreshFlicker([this, previewIdx, cardsSize](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 计算显示范围（当前选中卡片的前后各1张）
    int startIndex = max(0, previewIdx - 1);
    int endIndex = min(cardsSize - 1, previewIdx + 1);

    // 水平布局：每张卡片占70像素宽
    int cardWidth = 70;
    int startX = (212 - cardWidth * 3) / 2; // 居中对齐

    for (int i = startIndex; i <= endIndex; i++) {
      int x = startX + (i - startIndex) * cardWidth;
      int y = 20;

      // 绘制Logo（48x48）
      bool inverted = (i == previewIdx); // 选中的反色
      _drawCardLogo(d, x + 11, y, _cards[i]->getLogoPath(), inverted);

      // 绘制卡片名称（使用中文字体）
      String cardName = _cards[i]->getName();
      int16_t textWidth = ChineseFont::getStringWidth(cardName);
      int16_t textX = x + (48 - textWidth) / 2; // 居中对齐
      ChineseFont::drawString(d, textX, y + 67, cardName, GxEPD_BLACK);
    }
  });
}

void CardManager::_renderTransition() {
  Serial.println("[CardManager] Rendering transition");

  if (_switchPreviewIndex < 0 ||
      _switchPreviewIndex >= static_cast<int>(_cards.size())) {
    return;
  }

  Card *card = _cards[_switchPreviewIndex];
  String text = card->getCategory() + "/" + card->getName();

  _epd.refreshFull([text](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 使用中文字体渲染"分类/卡片名"
    int16_t textWidth = ChineseFont::getStringWidth(text);
    int16_t x = (212 - textWidth) / 2;
    int16_t y = 64; // 垂直居中（基线位置）

    ChineseFont::drawString(d, x, y, text, GxEPD_BLACK);
  });

  delay(1000);
}

void CardManager::_drawCardLogo(EPD_Class &d, int x, int y,
                                const String &logoPath, bool inverted) {
  // 从LittleFS加载Logo图标（48x48，288字节）
  if (!LittleFS.exists(logoPath)) {
    Serial.printf("[CardManager] Logo not found: %s\n", logoPath.c_str());
    d.drawRect(x, y, 48, 48, GxEPD_BLACK);
    return;
  }

  File file = LittleFS.open(logoPath, "r");
  if (!file) {
    Serial.printf("[CardManager] Failed to open logo: %s\n", logoPath.c_str());
    d.drawRect(x, y, 48, 48, GxEPD_BLACK);
    return;
  }

  uint8_t logoData[288];
  size_t bytesRead = file.read(logoData, 288);
  file.close();

  if (bytesRead != 288) {
    Serial.printf("[CardManager] Invalid logo size: %d bytes\n", bytesRead);
    d.drawRect(x, y, 48, 48, GxEPD_BLACK);
    return;
  }

  for (int dy = 0; dy < 48; dy++) {
    for (int dx = 0; dx < 48; dx++) {
      int byteIndex = (dy * 48 + dx) / 8;
      int bitIndex = (dy * 48 + dx) % 8;
      bool pixel = (logoData[byteIndex] >> bitIndex) & 0x01;

      if (inverted)
        pixel = !pixel;

      d.drawPixel(x + dx, y + dy, pixel ? GxEPD_BLACK : GxEPD_WHITE);
    }
  }
}

void CardManager::_flashWhite() {
  _epd.refreshFlicker([](EPD_Class &d) { d.fillScreen(GxEPD_WHITE); });
}

void CardManager::render(bool wifiConnected, int batteryLevel) {
  if (_state != STATE_NORMAL) {
    // 非正常模式下不渲染
    return;
  }

  if (_currentCardIndex < 0 ||
      _currentCardIndex >= static_cast<int>(_cards.size())) {
    Serial.printf("[CardManager] ERROR: Invalid current card index: %d (total "
                  "cards: %d)\n",
                  _currentCardIndex, _cards.size());
    return;
  }

  // 分配framebuffer（212x104，2808字节）
  const size_t FRAMEBUFFER_SIZE = 2808; // 27 bytes/row * 104 rows
  uint8_t *framebuffer = (uint8_t *)malloc(FRAMEBUFFER_SIZE);
  if (framebuffer == nullptr) {
    Serial.println("[CardManager] ERROR: Failed to allocate framebuffer");
    return;
  }

  // 清空framebuffer
  memset(framebuffer, 0, FRAMEBUFFER_SIZE);

  // 渲染当前卡片
  _cards[_currentCardIndex]->render(framebuffer, FRAMEBUFFER_SIZE);

  // 叠加状态栏
  _statusBar.overlay(framebuffer, 212, 104, wifiConnected, batteryLevel);

  // 刷新到墨水屏
  // TODO: 实现framebuffer到墨水屏的传输

  // 释放framebuffer
  free(framebuffer);
}
