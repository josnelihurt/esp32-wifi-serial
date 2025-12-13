#include "system_info.h"
#include <ArduinoLog.h>
namespace jrb::wifi_serial {
namespace {
String getSerialString() {
  uint64_t chipId = ESP.getEfuseMac();
  char serialStr[13];
  snprintf(serialStr, sizeof(serialStr), "%04X%08X", (uint16_t)(chipId >> 32),
           (uint32_t)chipId);
  return String(serialStr);
}
} // namespace

void SystemInfo::logSystemInformation() const {
  Log.traceln(__func__);
  Log.infoln(
R"(========================================
Welcome to ESP32-C3 Serial Bridge
========================================
Serial: %s
MAC: %s
Settings JSON: %s
OTA: %s
Ctrl+Y i: print system information
Ctrl+Y d: debug mode on/off
========================================
        )",
      getSerialString().c_str(), WiFi.macAddress().c_str(), preferencesStorage.serialize().c_str(), otaEnabled ? "Enabled" : "Disabled");
}

} // namespace jrb::wifi_serial
