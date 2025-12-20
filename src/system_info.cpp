#include "system_info.h"
#include "ArduinoLog.h"
#include "domain/config/preferences_storage.h"
#include <sstream>
namespace jrb::wifi_serial {
namespace {
std::string getSerialString() {
  uint64_t chipId = ESP.getEfuseMac();
  char serialStr[13];
  snprintf(serialStr, sizeof(serialStr), "%04X%08X", (uint16_t)(chipId >> 32),
           (uint32_t)chipId);
  return std::string(serialStr);
}
} // namespace

SystemInfo::SystemInfo(const PreferencesStorageDefault &storage, bool &ota)
    : preferencesStorage(storage), otaEnabled(ota) {}

std::string SystemInfo::getSpecialCharacterSettings() const {
  return std::string(
      R"(Special characters for USB interface:
Ctrl+Y i: print system information
Ctrl+Y d: debug mode on/off
Ctrl+Y b: tty02tty1 bridge on/off
Ctrl+Y n reset the device (operation can be cancelled by hitting any key within the countdown))");
}

std::string SystemInfo::getWelcomeString() const {
  std::string macAddress = WiFi.macAddress().c_str();
  std::string ipAddress = (WiFi.status() == WL_CONNECTED)
                              ? WiFi.localIP().toString().c_str()
                              : "Not connected";
  std::string ssid =
      (WiFi.status() == WL_CONNECTED) ? WiFi.SSID().c_str() : "Not configured";

  std::ostringstream result;
  result << "\n========================================\n"
         << "Welcome to ESP32-C3 Serial Bridge\n"
         << "========================================\n"
         << "Serial: " << getSerialString() << "\n"
         << "MAC: " << macAddress << "\n"
         << "Settings JSON: "
         << preferencesStorage.serialize(ipAddress, macAddress, ssid) << "\n"
         << "OTA: " << (otaEnabled ? "Enabled" : "Disabled") << "\n"
         << getSpecialCharacterSettings() << "\n"
         << "========================================\n";

  return result.str();
}

void SystemInfo::logSystemInformation() const {
  Log.traceln(__func__);
  std::string info = getWelcomeString();
  Log.infoln("%s", info.c_str());
}
} // namespace jrb::wifi_serial
