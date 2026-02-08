#include "Driver/EPD_Driver.h"
#include "Input/InputManager.h"
#include "Network/NetworkManager.h"
#include "PinConfig.h"
#include "Utils/ChineseFont.h"
#include "Version.h"

// 核心框架
#include "Core/Card.h"
#include "Core/CardManager.h"
#include "Core/ConfigManager.h"
#include "Core/Event.h"
#include "Core/EventQueue.h"
#include "Core/StatusBar.h"

// 网络
#include "Network/OTAManager.h"
#include "Network/WiFiProvisioning.h"

// 卡片
#include "Cards/SettingsCard.h"
#include "Cards/LuaCard.h"

// Lua引擎
#include "Lua/LuaEngine.h"
#include "Lua/LuaBindings.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <esp_task_wdt.h>
#include <memory>

// ==========================================
// 全局对象
// ==========================================
EPD_Driver epd;
InputManager input;
NetworkManager network;
ConfigManager config;
StatusBar statusBar;
EventQueue eventQueue;

std::unique_ptr<CardManager> cardManager;

// 网络工具
std::unique_ptr<WiFiProvisioning> wifiProvisioning;
std::unique_ptr<OTAManager> otaManager;

// 卡片对象
std::unique_ptr<SettingsCard> settingsCard;
std::vector<std::unique_ptr<LuaCard>> luaCards;

// ==========================================
// 状态变量
// ==========================================
bool g_systemReady = false;
bool g_wifiConnected = false;
int g_batteryLevel = 5; // 0-5段显示

// ==========================================
// 辅助函数
// ==========================================

void displayProgress(const char *message, int step, int total) {
  int progress = (step * 176) / total; // barWidth - 4 = 176

  epd.refreshPartial([message, step, total, progress](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题（Y=8）
    ChineseFont::drawString(d, 10, 8, "启动中", GxEPD_BLACK);

    // 进度条（Y=30，需要手动加上OFFSET_Y=18）
    int barWidth = 180;
    int barHeight = 20;
    int barX = 16;
    int barY = 30 + 18;  // 加上硬件偏移

    d.drawRect(barX, barY, barWidth, barHeight, GxEPD_BLACK);
    if (progress > 0) {
      d.fillRect(barX + 2, barY + 2, progress, barHeight - 4, GxEPD_BLACK);
    }

    // 步骤文本（Y=56）
    String stepText = "步骤 " + String(step) + "/" + String(total);
    ChineseFont::drawString(d, 10, 56, stepText, GxEPD_BLACK);

    // 状态消息（Y=74）
    ChineseFont::drawString(d, 10, 74, message, GxEPD_BLACK);
  });
}

void displayError(const char *message) {
  Serial.print("Error: ");
  Serial.println(message);

  epd.refreshFull([message](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题（Y=8）
    ChineseFont::drawString(d, 10, 8, "错误", GxEPD_BLACK);

    // 错误消息（Y=28）
    ChineseFont::drawString(d, 10, 28, message, GxEPD_BLACK);
  });
}

bool initLittleFS() {
  Serial.println("[LittleFS] Initializing...");

  if (!LittleFS.begin(true)) {
    Serial.println("[LittleFS] ERROR: Failed to mount");
    return false;
  }

  Serial.println("[LittleFS] Mounted successfully");

  // 显示文件系统信息
  size_t totalBytes = LittleFS.totalBytes();
  size_t usedBytes = LittleFS.usedBytes();
  Serial.printf("[LittleFS] Total: %d bytes, Used: %d bytes\n", totalBytes,
                usedBytes);

  return true;
}

bool connectWiFi() {
  String ssid = config.getWiFiSSID();
  String password = config.getWiFiPassword();

  if (ssid.isEmpty()) {
    Serial.println("[WiFi] No WiFi config found");
    return false;
  }

  Serial.printf("[WiFi] Connecting to: %s\n", ssid.c_str());

  // 配置WiFi
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(WIFI_PS_NONE);
  WiFi.setAutoReconnect(true);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  delay(500);

  // 连接WiFi
  network.begin(ssid.c_str(), password.c_str());

  if (!network.waitForConnection(15000)) {
    Serial.println("[WiFi] Connection failed");
    return false;
  }

  Serial.println("[WiFi] Connected successfully");
  Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());

  return true;
}

void initCards() {
  Serial.println("[Cards] Initializing cards...");

  // 创建WiFi配网工具
  wifiProvisioning =
      std::unique_ptr<WiFiProvisioning>(new WiFiProvisioning(epd, config));

  // 创建OTA管理器
  otaManager = std::unique_ptr<OTAManager>(new OTAManager(epd, config));

  // 创建设置卡片
  settingsCard = std::unique_ptr<SettingsCard>(
      new SettingsCard(epd, config, *wifiProvisioning, *otaManager));
  cardManager->registerCard(settingsCard.get());

  // 初始化Lua引擎
  LuaEngine& lua = LuaEngine::getInstance();
  if (!lua.begin(true)) {  // 使用PSRAM
    Serial.println("[ERROR] Failed to initialize Lua engine");
  } else {
    // 注册Lua API
    LuaBindings::registerAll(epd, config);

    // 扫描并加载所有Lua卡片
    File root = LittleFS.open("/cards");
    if (!root) {
      Serial.println("[ERROR] Failed to open /cards directory");
    } else if (!root.isDirectory()) {
      Serial.println("[ERROR] /cards is not a directory");
      root.close();
    } else {
      Serial.println("[Cards] Scanning /cards directory");
      File file = root.openNextFile();
      while (file) {
        if (!file.isDirectory()) {
          String filename = file.name();
          if (filename.endsWith(".lua")) {
            String scriptPath = "/cards/" + filename;
            std::unique_ptr<LuaCard> luaCard(new LuaCard(scriptPath, epd));
            if (luaCard->isLoaded()) {
              cardManager->registerCard(luaCard.get());
              luaCards.push_back(std::move(luaCard));
              Serial.printf("[Cards] Loaded Lua card: %s\n", filename.c_str());
            } else {
              Serial.printf("[Cards] Failed to load: %s (%s)\n",
                           filename.c_str(), luaCard->getError().c_str());
            }
          }
        }
        file.close();
        file = root.openNextFile();
      }
      root.close();
    }
  }

  Serial.printf("[Cards] Registered %d cards\n", cardManager->getCardCount());
}

// ==========================================
// Setup
// ==========================================

void setup() {
  // ==========================================
  // 关键：最优先设置 PWR_IO 保持供电！
  // ==========================================
  pinMode(PIN_PWR_IO, OUTPUT);
  digitalWrite(PIN_PWR_IO, HIGH);
  delay(100);

  // 配置 Task Watchdog Timer (30秒超时)
  esp_task_wdt_deinit();
  esp_task_wdt_init(30, true);
  esp_task_wdt_add(NULL);

  Serial.begin(115200);
  delay(500);

  Serial.println("========================================");
  Serial.println("  INFINITY TAG V2 - Lua Card Engine");
  Serial.println("========================================");
  Serial.println("[CRITICAL] PWR_IO set to HIGH - Power hold enabled");

  char versionStr[64];
  snprintf(versionStr, sizeof(versionStr), "Firmware: v%d.%d.%d.%d",
           VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_BUILD);
  Serial.println(versionStr);

  // Step 1: 初始化硬件
  displayProgress("初始化硬件", 1, 6);
  epd.begin();
  input.begin();
  Serial.println("[OK] Hardware initialized");

  // Step 2: 初始化LittleFS
  displayProgress("初始化文件系统", 2, 6);
  if (!initLittleFS()) {
    displayError("文件系统失败");
    while (1) {
      esp_task_wdt_reset();
      delay(1000);
    }
  }
  Serial.println("[OK] LittleFS initialized");

  // Step 3: 初始化配置管理器
  displayProgress("加载配置", 3, 6);
  if (!config.begin()) {
    displayError("配置加载失败");
    while (1) {
      esp_task_wdt_reset();
      delay(1000);
    }
  }
  Serial.println("[OK] Config loaded");

  // Step 4: 初始化核心组件
  displayProgress("初始化核心", 4, 6);
  statusBar.begin();
  cardManager = std::unique_ptr<CardManager>(new CardManager(epd, statusBar));
  cardManager->begin();
  Serial.println("[OK] Core initialized");

  // Step 5: 初始化卡片
  displayProgress("加载卡片", 5, 6);
  initCards();
  Serial.println("[OK] Cards loaded");

  // Step 6: 检查WiFi配置
  displayProgress("检查网络", 6, 6);

  if (config.isFirstBoot() || !config.hasWiFiConfig()) {
    Serial.println("[Setup] First boot or no WiFi config - entering settings");

    // 进入设置卡片（会自动进入配网模式）
    cardManager->setCurrentCard(0);
    g_systemReady = true;

    Serial.println("========================================");
    Serial.println("  SYSTEM READY (First Boot)");
    Serial.println("========================================");
    return;
  }

  // 尝试连接WiFi
  epd.hibernate();
  delay(500);

  g_wifiConnected = connectWiFi();

  epd.begin();
  delay(500);

  if (!g_wifiConnected) {
    Serial.println("[Setup] WiFi connection failed - offline mode");
  }

  // 显示就绪信息
  bool wifiStatus = g_wifiConnected;
  int cardCount = cardManager->getCardCount();
  String ipAddr = wifiStatus ? WiFi.localIP().toString() : "";

  epd.refreshFull([wifiStatus, cardCount, ipAddr](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题（Y=8）
    ChineseFont::drawString(d, 10, 8, "系统就绪", GxEPD_BLACK);

    // WiFi状态（Y=28）
    if (wifiStatus) {
      ChineseFont::drawString(d, 10, 28, "WiFi:", GxEPD_BLACK);
      ChineseFont::drawString(d, 80, 28, ipAddr, GxEPD_BLACK);
    } else {
      ChineseFont::drawString(d, 10, 28, "WiFi: 离线", GxEPD_BLACK);
    }

    // 卡片数量（Y=46）
    ChineseFont::drawString(d, 10, 46, "卡片:", GxEPD_BLACK);
    String cardCountStr = String(cardCount);
    ChineseFont::drawString(d, 80, 46, cardCountStr, GxEPD_BLACK);
  });

  delay(2000);

  // 设置默认卡片（如果有卡片的话）
  if (cardManager->getCardCount() > 0) {
    // 从配置中读取上次选中的卡片索引
    int lastCardIndex = config.getCurrentCardIndex();
    if (lastCardIndex >= 0 && lastCardIndex < cardManager->getCardCount()) {
      cardManager->setCurrentCard(lastCardIndex);
    } else {
      // 默认选择第一张卡片
      cardManager->setCurrentCard(0);
    }
    Serial.printf("[Setup] Set current card to index: %d\n",
                  cardManager->getCurrentCardIndex());
  }

  g_systemReady = true;

  Serial.println("========================================");
  Serial.println("  SYSTEM READY");
  Serial.println("========================================");
}

// ==========================================
// Main Loop
// ==========================================

void loop() {
  if (!g_systemReady) {
    esp_task_wdt_reset();
    delay(100);
    return;
  }

  // 1. 如果设置卡片正在配网，处理DNS和Web请求
  if (settingsCard && cardManager->getCurrentCardIndex() == 0) {
    settingsCard->update();
  }

  // 2. 更新输入状态并产生事件
  input.update(eventQueue);

  // 3. 处理事件队列
  cardManager->processEvents(eventQueue);

  // 4. 更新WiFi连接状态
  g_wifiConnected = network.isConnected();

  // 5. 更新电池电量（TODO: 实现电池电量检测）
  // g_batteryLevel = readBatteryLevel();

  // 6. 渲染当前卡片和状态栏
  // cardManager->render(g_wifiConnected, g_batteryLevel);

  // 7. 喂狗
  esp_task_wdt_reset();

  delay(10);
}
