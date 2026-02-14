#ifndef CARD_MANAGER_H
#define CARD_MANAGER_H

#include "Card.h"
#include "Driver/EPD_Driver.h"
#include "Driver/Layer.h"
#include "Event.h"
#include "EventQueue.h"
#include "StatusBar.h"
#include <vector>


/**
 * @brief 卡片管理器状态
 */
enum CardManagerState {
  STATE_NORMAL,       // 正常模式（当前卡片运行）
  STATE_SWITCHING,    // 切换模式（显示卡片选择界面）
  STATE_TRANSITIONING // 过渡动画（显示"分类/卡片名"）
};

/**
 * @brief 卡片管理器
 *
 * 管理卡片生命周期、切换逻辑、事件分发和渲染
 */
class CardManager {
public:
  CardManager(EPD_Driver &epd, StatusBar &statusBar);
  ~CardManager();

  /**
   * @brief 初始化卡片管理器
   * @return true 成功，false 失败
   */
  bool begin();

  /**
   * @brief 注册卡片
   * @param card 卡片指针（CardManager不负责释放内存）
   */
  void registerCard(Card *card);

  /**
   * @brief 设置当前卡片（通过索引）
   * @param index 卡片索引
   * @return true 成功，false 失败
   */
  bool setCurrentCard(int index);

  /**
   * @brief 获取当前卡片索引
   * @return 当前卡片索引
   */
  int getCurrentCardIndex() const { return _currentCardIndex; }

  /**
   * @brief 获取卡片数量
   * @return 卡片数量
   */
  int getCardCount() const { return _cards.size(); }

  /**
   * @brief 处理事件队列
   *
   * 从事件队列中取出事件并处理
   * 应该在主循环中调用
   */
  void processEvents(EventQueue &eventQueue);

  /**
   * @brief 渲染当前卡片和状态栏
   *
   * @param wifiConnected WiFi是否连接
   * @param batteryLevel 电池电量（0-5）
   */
  void render(bool wifiConnected, int batteryLevel);

  /**
   * @brief 获取当前状态
   * @return 当前状态
   */
  CardManagerState getState() const { return _state; }

private:
  EPD_Driver &_epd;
  StatusBar &_statusBar;

  std::vector<Card *> _cards; // 卡片列表
  int _currentCardIndex;      // 当前卡片索引
  CardManagerState _state;    // 当前状态

  // 切换模式相关
  uint32_t _longPressStartTime;  // 长按开始时间
  bool _isLongPressing;          // 是否正在长按
  int _switchPreviewIndex;       // 切换预览索引
  uint32_t _switchModeStartTime; // 切换模式开始时间

  // 常量
  static const uint32_t LONG_PRESS_DURATION = 1000; // 长按时长（毫秒）
  static const uint32_t SWITCH_MODE_TIMEOUT = 5000; // 切换模式超时（毫秒）
  static const int CARDS_PER_SCREEN = 3;            // 一屏显示的卡片数量

  /**
   * @brief 处理单个事件
   * @param event 要处理的事件
   */
  void _handleEvent(const Event &event);

  /**
   * @brief 处理正常模式下的事件
   * @param event 要处理的事件
   */
  void _handleNormalEvent(const Event &event);

  /**
   * @brief 处理切换模式下的事件
   * @param event 要处理的事件
   */
  void _handleSwitchingEvent(const Event &event);

  /**
   * @brief 进入切换模式
   */
  void _enterSwitchMode();

  /**
   * @brief 退出切换模式（切换到选中的卡片）
   */
  void _exitSwitchMode();

  /**
   * @brief 取消切换模式（返回原卡片）
   */
  void _cancelSwitchMode();

  /**
   * @brief 渲染卡片切换界面
   *
   * 显示一屏3张卡片，选中的反色显示
   */
  void _renderCardSwitchUI();

  /**
   * @brief 渲染过渡动画
   *
   * 显示"分类/卡片名"
   */
  void _renderTransition();

  /**
   * @brief 绘制卡片Logo
   *
   * @param d EPD显示对象引用
   * @param x Logo X坐标
   * @param y Logo Y坐标
   * @param logoPath Logo文件路径
   * @param inverted 是否反色显示
   */
  void _drawCardLogo(EPD_Class &d, int x, int y, const String &logoPath,
                     bool inverted);

  /**
   * @brief 全屏闪白
   */
  void _flashWhite();

  /**
   * @brief 检查切换模式是否超时
   */
  void _checkSwitchModeTimeout();
};

#endif // CARD_MANAGER_H
