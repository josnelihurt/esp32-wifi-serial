#include "system_info.h"
#include "domain/serial/serial_buffer_manager.h"
#include "ArduinoLog.h"
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
  String macAddress = WiFi.macAddress();
  String ipAddress = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString()
                                                   : "Not connected";
  String ssid = WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "Not configured";
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
      getSerialString().c_str(), macAddress.c_str(),
      preferencesStorage
          .serialize(ipAddress, macAddress, ssid)
          .c_str(),
      otaEnabled ? "Enabled" : "Disabled");
}

} // namespace jrb::wifi_serial
