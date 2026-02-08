#include "Lua/LuaEngine.h"
#include <LittleFS.h>

// 获取单例实例
LuaEngine& LuaEngine::getInstance() {
  static LuaEngine instance;
  return instance;
}

// 自定义内存分配器（使用PSRAM）
void* LuaEngine::psramAllocator(void* ud, void* ptr, size_t osize,
                                 size_t nsize) {
  (void)ud;
  (void)osize;

  if (nsize == 0) {
    // 释放内存
    if (ptr) {
      free(ptr);
    }
    return nullptr;
  } else {
    // 分配或重新分配内存
    void* newPtr = nullptr;

#ifdef BOARD_HAS_PSRAM
    // 优先使用PSRAM
    if (psramFound()) {
      newPtr = ps_realloc(ptr, nsize);
    }
#endif

    // PSRAM不可用或分配失败，降级到DRAM
    if (!newPtr) {
      newPtr = realloc(ptr, nsize);
    }

    return newPtr;
  }
}

// 超时钩子函数
void LuaEngine::timeoutHook(lua_State* L, lua_Debug* ar) {
  (void)ar;
  LuaEngine& engine = getInstance();

  // 检查是否超时
  if (millis() - engine.timeoutStart > engine.timeoutMs) {
    luaL_error(L, "Script execution timeout (%u ms)", engine.timeoutMs);
  }
}

// 禁用危险函数
void LuaEngine::disableDangerousFunctions() {
  if (!L)
    return;

  // 禁用文件操作函数
  lua_pushnil(L);
  lua_setglobal(L, "dofile");
  lua_pushnil(L);
  lua_setglobal(L, "loadfile");
  lua_pushnil(L);
  lua_setglobal(L, "require");

  // 禁用包管理
  lua_pushnil(L);
  lua_setglobal(L, "package");

  Serial.println("[LuaEngine] Dangerous functions disabled");
}

// 初始化Lua虚拟机
bool LuaEngine::begin(bool usePSRAM) {
  if (L) {
    Serial.println("[LuaEngine] Already initialized");
    return true;
  }

  this->usePSRAM = usePSRAM;

  // 创建Lua状态机
  if (usePSRAM && psramFound()) {
    L = lua_newstate(psramAllocator, nullptr);
    Serial.println("[LuaEngine] Using PSRAM for Lua heap");
  } else {
    L = luaL_newstate();
    Serial.println("[LuaEngine] Using DRAM for Lua heap");
  }

  if (!L) {
    lastError = "Failed to create Lua state";
    Serial.printf("[LuaEngine] ERROR: %s\n", lastError.c_str());
    return false;
  }

  // 加载必要的标准库
  luaL_openlibs(L);  // 加载所有标准库
  Serial.println("[LuaEngine] Standard libraries loaded");

  // 禁用危险函数
  disableDangerousFunctions();

  // 设置超时钩子（每100条指令检查一次）
  lua_sethook(L, timeoutHook, LUA_MASKCOUNT, 100);

  Serial.printf("[LuaEngine] Initialized successfully (Memory: %u bytes)\n",
                getMemoryUsage());
  return true;
}

// 从LittleFS加载Lua脚本
bool LuaEngine::loadScript(const String& scriptPath) {
  if (!L) {
    lastError = "Lua engine not initialized";
    Serial.printf("[LuaEngine] ERROR: %s\n", lastError.c_str());
    return false;
  }

  // 打开文件
  File file = LittleFS.open(scriptPath, "r");
  if (!file) {
    lastError = "Failed to open script: " + scriptPath;
    Serial.printf("[LuaEngine] ERROR: %s\n", lastError.c_str());
    return false;
  }

  // 读取文件内容
  String scriptContent = file.readString();
  file.close();

  if (scriptContent.length() == 0) {
    lastError = "Script is empty: " + scriptPath;
    Serial.printf("[LuaEngine] ERROR: %s\n", lastError.c_str());
    return false;
  }

  // 加载脚本
  timeoutStart = millis();
  int result = luaL_loadstring(L, scriptContent.c_str());
  if (result != LUA_OK) {
    lastError = lua_tostring(L, -1);
    lua_pop(L, 1);
    Serial.printf("[LuaEngine] ERROR: Failed to load script: %s\n",
                  lastError.c_str());
    return false;
  }

  // 执行脚本（初始化全局变量和函数）
  result = lua_pcall(L, 0, 0, 0);
  if (result != LUA_OK) {
    lastError = lua_tostring(L, -1);
    lua_pop(L, 1);
    Serial.printf("[LuaEngine] ERROR: Failed to execute script: %s\n",
                  lastError.c_str());
    return false;
  }

  Serial.printf("[LuaEngine] Script loaded: %s (%u bytes)\n",
                scriptPath.c_str(), scriptContent.length());
  return true;
}

// 调用Lua函数
bool LuaEngine::callFunction(const char* funcName, int args) {
  if (!L) {
    lastError = "Lua engine not initialized";
    return false;
  }

  // 获取函数
  lua_getglobal(L, funcName);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1 + args);  // 弹出函数和参数
    // 函数不存在不算错误（可能是可选回调）
    return true;
  }

  // 如果有参数，需要将函数移到参数下面
  if (args > 0) {
    lua_insert(L, -(args + 1));
  }

  // 调用函数
  timeoutStart = millis();
  int result = lua_pcall(L, args, 0, 0);
  if (result != LUA_OK) {
    lastError = lua_tostring(L, -1);
    lua_pop(L, 1);
    Serial.printf("[LuaEngine] ERROR: Failed to call %s: %s\n", funcName,
                  lastError.c_str());
    return false;
  }

  return true;
}

// 设置全局字符串变量
void LuaEngine::setGlobalString(const char* name, const String& value) {
  if (!L)
    return;
  lua_pushstring(L, value.c_str());
  lua_setglobal(L, name);
}

// 设置全局整数变量
void LuaEngine::setGlobalInt(const char* name, int value) {
  if (!L)
    return;
  lua_pushinteger(L, value);
  lua_setglobal(L, name);
}

// 设置全局浮点数变量
void LuaEngine::setGlobalNumber(const char* name, double value) {
  if (!L)
    return;
  lua_pushnumber(L, value);
  lua_setglobal(L, name);
}

// 获取全局字符串变量
String LuaEngine::getGlobalString(const char* name) {
  if (!L)
    return "";
  lua_getglobal(L, name);
  String result = "";
  if (lua_isstring(L, -1)) {
    result = lua_tostring(L, -1);
  }
  lua_pop(L, 1);
  return result;
}

// 获取全局整数变量
int LuaEngine::getGlobalInt(const char* name) {
  if (!L)
    return 0;
  lua_getglobal(L, name);
  int result = 0;
  if (lua_isinteger(L, -1)) {
    result = lua_tointeger(L, -1);
  }
  lua_pop(L, 1);
  return result;
}

// 获取全局浮点数变量
double LuaEngine::getGlobalNumber(const char* name) {
  if (!L)
    return 0.0;
  lua_getglobal(L, name);
  double result = 0.0;
  if (lua_isnumber(L, -1)) {
    result = lua_tonumber(L, -1);
  }
  lua_pop(L, 1);
  return result;
}

// 重置Lua虚拟机
void LuaEngine::reset() {
  if (L) {
    lua_close(L);
    L = nullptr;
    Serial.println("[LuaEngine] Reset");
  }
}

// 获取Lua内存使用量
size_t LuaEngine::getMemoryUsage() {
  if (!L)
    return 0;
  return lua_gc(L, LUA_GCCOUNT, 0) * 1024 + lua_gc(L, LUA_GCCOUNTB, 0);
}
