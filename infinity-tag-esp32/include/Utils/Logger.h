#pragma once
#include <Arduino.h>

// Uncomment to enable debug output
#define ENABLE_DEBUG_LOGGING

#ifdef ENABLE_DEBUG_LOGGING
#define LOG_DEBUG(msg) Serial.println(msg)
#define LOG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define LOG_DEBUG(msg)
#define LOG_PRINTF(...)
#endif
