/**
 * @file arduino_compat.h
 * @brief Arduino compatibility layer for non-ESP platforms (testing).
 *
 * Provides mock implementations of Arduino-specific functions and types
 * to allow code compilation in native test environments.
 */

#pragma once

#ifndef ESP_PLATFORM

#include <cstdint>
#include <cstdlib>
#include <chrono>

// Arduino byte type
using byte = uint8_t;

namespace {
// Simple mock for millis() - returns milliseconds since epoch
inline unsigned long millis() {
  auto now = std::chrono::steady_clock::now();
  auto duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// Simple mock for delay() - does nothing in tests (can be enhanced if needed)
inline void delay(unsigned long ms) {
  (void)ms;
  // In tests, we don't actually delay
}

// Simple mock for random() - uses std::rand with max value
inline long random(long max) {
  return std::rand() % max;
}

// Simple mock for random() with min/max - uses std::rand
inline long random(long min, long max) {
  return min + (std::rand() % (max - min));
}
}

#endif // !ESP_PLATFORM
