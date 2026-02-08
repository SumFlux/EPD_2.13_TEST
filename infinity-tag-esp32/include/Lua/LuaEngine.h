#ifndef LUA_ENGINE_H
#define LUA_ENGINE_H

#include <Arduino.h>
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

/**
 * @brief Lua虚拟机管理类（单例模式）
 *
 * 功能：
 * - 管理Lua虚拟机生命周期
 * - 使用PSRAM作为Lua堆（可选）
 * - 脚本加载和执行
 * - 超时看门狗（防止死循环）
 * - 错误处理
 */
class LuaEngine {
public:
  /**
   * @brief 获取单例实例
   */
  static LuaEngine& getInstance();

  /**
   * @brief 初始化Lua虚拟机
   * @param usePSRAM 是否使用PSRAM作为Lua堆
   * @return 成功返回true
   */
  bool begin(bool usePSRAM = true);

  /**
   * @brief 从LittleFS加载Lua脚本
   * @param scriptPath 脚本路径（如 "/cards/image.lua"）
   * @return 成功返回true
   */
  bool loadScript(const String& scriptPath);

  /**
   * @brief 调用Lua函数
   * @param funcName 函数名
   * @param args 参数数量（已压入栈）
   * @return 成功返回true
   */
  bool callFunction(const char* funcName, int args = 0);

  /**
   * @brief 设置全局字符串变量
   */
  void setGlobalString(const char* name, const String& value);

  /**
   * @brief 设置全局整数变量
   */
  void setGlobalInt(const char* name, int value);

  /**
   * @brief 设置全局浮点数变量
   */
  void setGlobalNumber(const char* name, double value);

  /**
   * @brief 获取全局字符串变量
   */
  String getGlobalString(const char* name);

  /**
   * @brief 获取全局整数变量
   */
  int getGlobalInt(const char* name);

  /**
   * @brief 获取全局浮点数变量
   */
  double getGlobalNumber(const char* name);

  /**
   * @brief 获取最后一次错误信息
   */
  String getLastError() const { return lastError; }

  /**
   * @brief 重置Lua虚拟机（关闭并重新初始化）
   */
  void reset();

  /**
   * @brief 获取Lua状态机（供绑定层使用）
   */
  lua_State* getState() { return L; }

  /**
   * @brief 设置脚本执行超时时间（毫秒）
   */
  void setTimeout(uint32_t ms) { timeoutMs = ms; }

  /**
   * @brief 获取Lua内存使用量（字节）
   */
  size_t getMemoryUsage();

  /**
   * @brief 检查Lua虚拟机是否已初始化
   */
  bool isInitialized() const { return L != nullptr; }

private:
  LuaEngine() : L(nullptr), usePSRAM(false), timeoutMs(5000) {}
  ~LuaEngine() { reset(); }

  // 禁止拷贝和赋值
  LuaEngine(const LuaEngine&) = delete;
  LuaEngine& operator=(const LuaEngine&) = delete;

  /**
   * @brief 自定义内存分配器（使用PSRAM）
   */
  static void* psramAllocator(void* ud, void* ptr, size_t osize, size_t nsize);

  /**
   * @brief 超时钩子函数
   */
  static void timeoutHook(lua_State* L, lua_Debug* ar);

  /**
   * @brief 禁用危险函数
   */
  void disableDangerousFunctions();

  lua_State* L;
  String lastError;
  bool usePSRAM;
  uint32_t timeoutMs;
  uint32_t timeoutStart;
};

#endif // LUA_ENGINE_H
