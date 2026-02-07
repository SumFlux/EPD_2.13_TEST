#include "Core/ConfigManager.h"

// NVS 命名空间定义
const char *ConfigManager::NAMESPACE_WIFI = "wifi";
const char *ConfigManager::NAMESPACE_DEVICE = "device";
const char *ConfigManager::NAMESPACE_SETTINGS = "settings";
const char *ConfigManager::NAMESPACE_CARDS = "cards";
const char *ConfigManager::NAMESPACE_SYSTEM = "system";

ConfigManager::ConfigManager() {}

ConfigManager::~ConfigManager() { _prefs.end(); }

bool ConfigManager::begin() {
  // 初始化成功
  Serial.println("[ConfigManager] Initialized");
  return true;
}

// ==========================================
// WiFi 配置
// ==========================================

String ConfigManager::getWiFiSSID() {
  _prefs.begin(NAMESPACE_WIFI, true); // 只读模式
  String ssid = _prefs.getString("ssid", "");
  _prefs.end();
  return ssid;
}

void ConfigManager::setWiFiSSID(const String &ssid) {
  _prefs.begin(NAMESPACE_WIFI, false); // 读写模式
  _prefs.putString("ssid", ssid);
  _prefs.end();
  Serial.printf("[ConfigManager] WiFi SSID set: %s\n", ssid.c_str());
}

String ConfigManager::getWiFiPassword() {
  _prefs.begin(NAMESPACE_WIFI, true);
  String password = _prefs.getString("password", "");
  _prefs.end();
  return password;
}

void ConfigManager::setWiFiPassword(const String &password) {
  _prefs.begin(NAMESPACE_WIFI, false);
  _prefs.putString("password", password);
  _prefs.end();
  Serial.println("[ConfigManager] WiFi password set");
}

bool ConfigManager::hasWiFiConfig() {
  String ssid = getWiFiSSID();
  return !ssid.isEmpty();
}

// ==========================================
// 设备配置
// ==========================================

String ConfigManager::getDeviceID() {
  _prefs.begin(NAMESPACE_DEVICE, true);
  String deviceId = _prefs.getString("device_id", "0BFB78");
  _prefs.end();
  return deviceId;
}

void ConfigManager::setDeviceID(const String &deviceId) {
  _prefs.begin(NAMESPACE_DEVICE, false);
  _prefs.putString("device_id", deviceId);
  _prefs.end();
  Serial.printf("[ConfigManager] Device ID set: %s\n", deviceId.c_str());
}

String ConfigManager::getDevicePassword() {
  _prefs.begin(NAMESPACE_DEVICE, true);
  String password = _prefs.getString("device_pwd", "123456");
  _prefs.end();
  return password;
}

void ConfigManager::setDevicePassword(const String &password) {
  _prefs.begin(NAMESPACE_DEVICE, false);
  _prefs.putString("device_pwd", password);
  _prefs.end();
  Serial.println("[ConfigManager] Device password set");
}

String ConfigManager::getAPIBaseURL() {
  _prefs.begin(NAMESPACE_DEVICE, true);
  String url = _prefs.getString("api_url", "http://192.168.31.57:8001");
  _prefs.end();
  return url;
}

void ConfigManager::setAPIBaseURL(const String &url) {
  _prefs.begin(NAMESPACE_DEVICE, false);
  _prefs.putString("api_url", url);
  _prefs.end();
  Serial.printf("[ConfigManager] API URL set: %s\n", url.c_str());
}

// ==========================================
// 用户设置
// ==========================================

bool ConfigManager::getSoundEnabled() {
  _prefs.begin(NAMESPACE_SETTINGS, true);
  bool enabled = _prefs.getBool("sound", true);
  _prefs.end();
  return enabled;
}

void ConfigManager::setSoundEnabled(bool enabled) {
  _prefs.begin(NAMESPACE_SETTINGS, false);
  _prefs.putBool("sound", enabled);
  _prefs.end();
  Serial.printf("[ConfigManager] Sound enabled: %d\n", enabled);
}

bool ConfigManager::getVibrateEnabled() {
  _prefs.begin(NAMESPACE_SETTINGS, true);
  bool enabled = _prefs.getBool("vibrate", true);
  _prefs.end();
  return enabled;
}

void ConfigManager::setVibrateEnabled(bool enabled) {
  _prefs.begin(NAMESPACE_SETTINGS, false);
  _prefs.putBool("vibrate", enabled);
  _prefs.end();
  Serial.printf("[ConfigManager] Vibrate enabled: %d\n", enabled);
}

// ==========================================
// 卡片配置
// ==========================================

std::vector<CardConfig> ConfigManager::getCardList() {
  std::vector<CardConfig> cards;

  _prefs.begin(NAMESPACE_CARDS, true);
  int count = _prefs.getInt("count", 0);

  for (int i = 0; i < count; i++) {
    CardConfig card;
    card.name = _prefs.getString(_getCardKey(i, "name").c_str(), "");
    card.category = _prefs.getString(_getCardKey(i, "cat").c_str(), "");
    card.scriptPath = _prefs.getString(_getCardKey(i, "script").c_str(), "");
    card.logoPath = _prefs.getString(_getCardKey(i, "logo").c_str(), "");
    card.order = _prefs.getInt(_getCardKey(i, "order").c_str(), i);
    card.enabled = _prefs.getBool(_getCardKey(i, "enabled").c_str(), true);

    if (!card.name.isEmpty()) {
      cards.push_back(card);
    }
  }

  _prefs.end();

  Serial.printf("[ConfigManager] Loaded %d cards\n", cards.size());
  return cards;
}

void ConfigManager::updateCardList(const std::vector<CardConfig> &cards) {
  _prefs.begin(NAMESPACE_CARDS, false);

  // 清空旧数据
  _prefs.clear();

  // 保存新数据
  _prefs.putInt("count", cards.size());

  for (size_t i = 0; i < cards.size(); i++) {
    const CardConfig &card = cards[i];
    _prefs.putString(_getCardKey(i, "name").c_str(), card.name);
    _prefs.putString(_getCardKey(i, "cat").c_str(), card.category);
    _prefs.putString(_getCardKey(i, "script").c_str(), card.scriptPath);
    _prefs.putString(_getCardKey(i, "logo").c_str(), card.logoPath);
    _prefs.putInt(_getCardKey(i, "order").c_str(), card.order);
    _prefs.putBool(_getCardKey(i, "enabled").c_str(), card.enabled);
  }

  _prefs.end();

  Serial.printf("[ConfigManager] Updated card list: %d cards\n", cards.size());
}

void ConfigManager::addCard(const CardConfig &card) {
  std::vector<CardConfig> cards = getCardList();
  cards.push_back(card);
  updateCardList(cards);
}

void ConfigManager::removeCard(const String &name) {
  std::vector<CardConfig> cards = getCardList();
  cards.erase(
      std::remove_if(cards.begin(), cards.end(),
                     [&name](const CardConfig &c) { return c.name == name; }),
      cards.end());
  updateCardList(cards);
}

int ConfigManager::getCardCount() {
  _prefs.begin(NAMESPACE_CARDS, true);
  int count = _prefs.getInt("count", 0);
  _prefs.end();
  return count;
}

// ==========================================
// 系统配置
// ==========================================

int ConfigManager::getCurrentCardIndex() {
  _prefs.begin(NAMESPACE_SYSTEM, true);
  int index = _prefs.getInt("card_index", 0);
  _prefs.end();
  return index;
}

void ConfigManager::setCurrentCardIndex(int index) {
  _prefs.begin(NAMESPACE_SYSTEM, false);
  _prefs.putInt("card_index", index);
  _prefs.end();
}

bool ConfigManager::isFirstBoot() {
  _prefs.begin(NAMESPACE_SYSTEM, true);
  bool firstBoot = _prefs.getBool("first_boot", true);
  _prefs.end();
  return firstBoot;
}

void ConfigManager::setFirstBoot(bool firstBoot) {
  _prefs.begin(NAMESPACE_SYSTEM, false);
  _prefs.putBool("first_boot", firstBoot);
  _prefs.end();
}

void ConfigManager::factoryReset() {
  Serial.println("[ConfigManager] Factory reset...");

  // 清空所有命名空间
  _prefs.begin(NAMESPACE_WIFI, false);
  _prefs.clear();
  _prefs.end();

  _prefs.begin(NAMESPACE_DEVICE, false);
  _prefs.clear();
  _prefs.end();

  _prefs.begin(NAMESPACE_SETTINGS, false);
  _prefs.clear();
  _prefs.end();

  _prefs.begin(NAMESPACE_CARDS, false);
  _prefs.clear();
  _prefs.end();

  _prefs.begin(NAMESPACE_SYSTEM, false);
  _prefs.clear();
  _prefs.end();

  Serial.println("[ConfigManager] Factory reset completed");
}

// ==========================================
// 辅助方法
// ==========================================

String ConfigManager::_getCardKey(int index, const char *suffix) {
  char key[32];
  snprintf(key, sizeof(key), "c%d_%s", index, suffix);
  return String(key);
}
