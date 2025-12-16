#include "system_info.h"
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

String SystemInfo::getSpecialCharacterSettings() const {
  return String(
      R"(Special characters for USB interface:
Ctrl+Y i: print system information
Ctrl+Y d: debug mode on/off
Ctrl+Y b: tty02tty1 bridge on/off
Ctrl+Y n reset the device (operation can be cancelled by hitting any key within the countdown))");
}

String SystemInfo::getWelcomeString() const {
  String macAddress = WiFi.macAddress();
  String ipAddress = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString()
                                                     : "Not connected";
  String ssid =
      (WiFi.status() == WL_CONNECTED) ? WiFi.SSID() : "Not configured";

  String result = "\n========================================\n";
  result += "Welcome to ESP32-C3 Serial Bridge\n";
  result += "========================================\n";
  result += "Serial: " + getSerialString() + "\n";
  result += "MAC: " + macAddress + "\n";
  result += "Settings JSON: " +
            preferencesStorage.serialize(ipAddress, macAddress, ssid) + "\n";
  result += "OTA: " + String(otaEnabled ? "Enabled" : "Disabled") + "\n";
  result += getSpecialCharacterSettings() + "\n";
  result += "========================================\n";

  return result;
}

void SystemInfo::logSystemInformation() const {
  Log.traceln(__func__);
  String info = getWelcomeString();
  Log.infoln("%s", info.c_str());
}
} // namespace jrb::wifi_serial
