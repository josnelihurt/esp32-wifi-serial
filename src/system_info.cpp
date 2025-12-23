#include "system_info.h"
#include "domain/config/preferences_storage.h"
#include "infrastructure/logging/logger.h"
#include "infrastructure/types.hpp"
#include <sstream>

#ifdef ESP_PLATFORM
#include <Arduino.h>
#include <WiFi.h>
#endif
namespace jrb::wifi_serial {
namespace {
#ifdef ESP_PLATFORM
types::string getSerialString() {
  uint64_t chipId = ESP.getEfuseMac();
  char serialStr[13];
  snprintf(serialStr, sizeof(serialStr), "%04X%08X", (uint16_t)(chipId >> 32),
           (uint32_t)chipId);
  return types::string(serialStr);
}
types::string getMacAddress() { return WiFi.macAddress().c_str(); }
types::string getIpAddress() {
  return (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString().c_str()
                                         : "Not connected";
}
types::string getSsid() {
  return (WiFi.status() == WL_CONNECTED) ? WiFi.SSID().c_str()
                                         : "Not configured";
}
#else
// TODO: This section needs a proper mock implementation
types::string getSerialString() { return "TEST12345"; }
types::string getMacAddress() { return "AA:BB:CC:DD:EE:FF"; }
types::string getIpAddress() { return "192.168.1.1"; }
types::string getSsid() { return "TEST_SSID"; }
#endif
} // namespace

SystemInfo::SystemInfo(const PreferencesStorage &storage, bool &ota)
    : preferencesStorage(storage), otaEnabled(ota) {}

types::string SystemInfo::getSpecialCharacterSettings() const {
  return types::string(
      R"(Special characters for USB interface:
Ctrl+Y i: print system information
Ctrl+Y d: debug mode on/off
Ctrl+Y b: tty02tty1 bridge on/off
Ctrl+Y n reset the device (operation can be cancelled by hitting any key within the countdown))");
}

types::string SystemInfo::getWelcomeString() const {
  std::ostringstream result;
  result << "\n========================================\n"
         << "Welcome to ESP32-C3 Serial Bridge\n"
         << "========================================\n"
         << "Serial: " << getSerialString() << "\n"
         << "Settings JSON: "
         << preferencesStorage.serialize(getIpAddress(), getMacAddress(),
                                         getSsid())
         << "\n"
         << "OTA: " << (otaEnabled ? "Enabled" : "Disabled") << "\n"
         << getSpecialCharacterSettings() << "\n"
         << "========================================\n";

  return result.str();
}

void SystemInfo::logSystemInformation() const {
  LOG_DEBUG(__func__);
  types::string info = getWelcomeString();
  LOG_INFO("%s", info.c_str());
}
} // namespace jrb::wifi_serial
