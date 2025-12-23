#pragma once

#ifdef ESP_PLATFORM
// ESP32 Platform - Real hardware calls
#include <Arduino.h>
#else
// Test Platform - Mock implementation
#include <cstdint>
#endif

namespace jrb::wifi_serial {

#ifdef ESP_PLATFORM
class HardwareResetPolicy {
public:
  void delay(uint32_t ms) { ::delay(ms); }
  bool isSerialDataAvailable() { return Serial.available() > 0; }
  void readSerialData() { Serial.read(); }
  void resetDevice() { ESP.restart(); }
};

#else
class HardwareResetPolicy {
private:
  uint32_t totalDelayMs;
  bool serialDataAvailable;
  bool resetCalled;

public:
  HardwareResetPolicy()
      : totalDelayMs(0), serialDataAvailable(false), resetCalled(false) {}

  void delay(uint32_t ms) { totalDelayMs += ms; }
  bool isSerialDataAvailable() { return serialDataAvailable; }
  void readSerialData() { serialDataAvailable = false; }
  void resetDevice() { resetCalled = true; }

  // Test helpers
  void setSerialDataAvailable(bool available) {
    serialDataAvailable = available;
  }
  uint32_t getTotalDelay() const { return totalDelayMs; }
  bool wasResetCalled() const { return resetCalled; }
  void reset() {
    totalDelayMs = 0;
    serialDataAvailable = false;
    resetCalled = false;
  }
};

#endif

} // namespace jrb::wifi_serial
