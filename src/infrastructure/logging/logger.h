#pragma once

/**
 * @file logger.h
 * @brief Centralized logging abstraction for ESP32 and testing environments
 *
 * This header provides a unified logging interface that works across different
 * platforms:
 * - ESP32: Uses ArduinoLog library
 * - Testing/Native: Uses standard printf to stdout
 *
 * All platform-specific #ifdef directives are contained in this single file,
 * keeping the rest of the codebase clean.
 *
 * Usage:
 *   #include "infrastructure/logging/logger.h"
 *
 *   LOG_INFO("Device initialized with name: %s", deviceName.c_str());
 *   LOG_WARN("Low memory: %d bytes remaining", freeMemory);
 *   LOG_ERROR("Failed to connect to broker: %s", error);
 *   LOG_DEBUG("Current state: %d", state);
 */

#ifdef ESP_PLATFORM
// ============================================================================
// ESP32 Platform - Use ArduinoLog
// ============================================================================
#include <ArduinoLog.h>

#define LOG_VERBOSE(...) Log.verboseln(__VA_ARGS__)
#define LOG_DEBUG(...) Log.traceln(__VA_ARGS__)
#define LOG_INFO(...) Log.infoln(__VA_ARGS__)
#define LOG_WARN(...) Log.warningln(__VA_ARGS__)
#define LOG_ERROR(...) Log.errorln(__VA_ARGS__)
#define LOG_FATAL(...) Log.fatalln(__VA_ARGS__)

// Optional: Raw log without newline (for building multi-part messages)
#define LOG_VERBOSE_RAW(...) Log.verbose(__VA_ARGS__)
#define LOG_DEBUG_RAW(...) Log.trace(__VA_ARGS__)
#define LOG_INFO_RAW(...) Log.info(__VA_ARGS__)
#define LOG_WARN_RAW(...) Log.warning(__VA_ARGS__)
#define LOG_ERROR_RAW(...) Log.error(__VA_ARGS__)
#define LOG_FATAL_RAW(...) Log.fatal(__VA_ARGS__)

#else
// ============================================================================
// Test/Native Platform - Use printf
// ============================================================================
#include <cstdio>

// Helper macro to add timestamp (optional, can be removed for simpler output)
#define LOG_PREFIX(level) printf("[%s] ", level)

#define LOG_VERBOSE(...)                                                       \
  do {                                                                         \
    LOG_PREFIX("VERBOSE");                                                     \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
    fflush(stdout);                                                            \
  } while (0)

#define LOG_DEBUG(...)                                                         \
  do {                                                                         \
    LOG_PREFIX("DEBUG");                                                       \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
    fflush(stdout);                                                            \
  } while (0)

#define LOG_INFO(...)                                                          \
  do {                                                                         \
    LOG_PREFIX("INFO");                                                        \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
    fflush(stdout);                                                            \
  } while (0)

#define LOG_WARN(...)                                                          \
  do {                                                                         \
    LOG_PREFIX("WARN");                                                        \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
    fflush(stdout);                                                            \
  } while (0)

#define LOG_ERROR(...)                                                         \
  do {                                                                         \
    LOG_PREFIX("ERROR");                                                       \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
    fflush(stdout);                                                            \
  } while (0)

#define LOG_FATAL(...)                                                         \
  do {                                                                         \
    LOG_PREFIX("FATAL");                                                       \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
    fflush(stdout);                                                            \
  } while (0)

// Raw versions without newline
#define LOG_VERBOSE_RAW(...)                                                   \
  do {                                                                         \
    printf(__VA_ARGS__);                                                       \
    fflush(stdout);                                                            \
  } while (0)

#define LOG_DEBUG_RAW(...)                                                     \
  do {                                                                         \
    printf(__VA_ARGS__);                                                       \
    fflush(stdout);                                                            \
  } while (0)

#define LOG_INFO_RAW(...)                                                      \
  do {                                                                         \
    printf(__VA_ARGS__);                                                       \
    fflush(stdout);                                                            \
  } while (0)

#define LOG_WARN_RAW(...)                                                      \
  do {                                                                         \
    printf(__VA_ARGS__);                                                       \
    fflush(stdout);                                                            \
  } while (0)

#define LOG_ERROR_RAW(...)                                                     \
  do {                                                                         \
    printf(__VA_ARGS__);                                                       \
    fflush(stdout);                                                            \
  } while (0)

#define LOG_FATAL_RAW(...)                                                     \
  do {                                                                         \
    printf(__VA_ARGS__);                                                       \
    fflush(stdout);                                                            \
  } while (0)

#endif

// ============================================================================
// Compile-Time Debug Control
// ============================================================================

// DISABLE_LOGGING: Completely remove ALL logging (maximum flash savings)
// Define DISABLE_LOGGING in platformio.ini build_flags to disable all logs
#ifdef DISABLE_LOGGING
#undef LOG_VERBOSE
#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_WARN
#undef LOG_ERROR
#undef LOG_FATAL
#undef LOG_VERBOSE_RAW
#undef LOG_DEBUG_RAW
#undef LOG_INFO_RAW
#undef LOG_WARN_RAW
#undef LOG_ERROR_RAW
#undef LOG_FATAL_RAW
#define LOG_VERBOSE(...) ((void)0)
#define LOG_DEBUG(...) ((void)0)
#define LOG_INFO(...) ((void)0)
#define LOG_WARN(...) ((void)0)
#define LOG_ERROR(...) ((void)0)
#define LOG_FATAL(...) ((void)0)
#define LOG_VERBOSE_RAW(...) ((void)0)
#define LOG_DEBUG_RAW(...) ((void)0)
#define LOG_INFO_RAW(...) ((void)0)
#define LOG_WARN_RAW(...) ((void)0)
#define LOG_ERROR_RAW(...) ((void)0)
#define LOG_FATAL_RAW(...) ((void)0)
#endif

// In release builds, completely remove debug and verbose logs
#ifdef NDEBUG
#ifndef DISABLE_LOGGING // Only apply if DISABLE_LOGGING isn't already set
#undef LOG_VERBOSE
#undef LOG_DEBUG
#undef LOG_VERBOSE_RAW
#undef LOG_DEBUG_RAW
#define LOG_VERBOSE(...) ((void)0)
#define LOG_DEBUG(...) ((void)0)
#define LOG_VERBOSE_RAW(...) ((void)0)
#define LOG_DEBUG_RAW(...) ((void)0)
#endif
#endif

// ============================================================================
// Convenience Macros
// ============================================================================

// Log with function name and line number (useful for debugging)
#ifndef DISABLE_LOGGING
#define LOG_TRACE() LOG_DEBUG("%s:%d - %s()", __FILE__, __LINE__, __func__)
#else
#define LOG_TRACE() ((void)0)
#endif

// Conditional logging
#ifndef DISABLE_LOGGING
#define LOG_IF(condition, level, ...)                                          \
  do {                                                                         \
    if (condition) {                                                           \
      LOG_##level(__VA_ARGS__);                                                \
    }                                                                          \
  } while (0)
#else
#define LOG_IF(condition, level, ...) ((void)0)
#endif

// Example: LOG_IF(count > 100, WARN, "Count exceeded threshold: %d", count)
