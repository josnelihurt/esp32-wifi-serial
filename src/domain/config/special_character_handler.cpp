#include "special_character_handler.h"
#include "ArduinoLog.h"

namespace jrb::wifi_serial {

SpecialCharacterHandler::SpecialCharacterHandler(
    SystemInfo &systemInfo, PreferencesStorage &preferencesStorage)
    : systemInfo(systemInfo), preferencesStorage(preferencesStorage),
      specialCharacterMode(false) {}

bool SpecialCharacterHandler::handle(char c) {
  if (c == CMD_PREFIX) {
    specialCharacterMode = true;
    Log.infoln("%s", systemInfo.getSpecialCharacterSettings().c_str());
    return true;
  }
  if (!specialCharacterMode)
    return false;
  specialCharacterMode = false;
  switch (c) {
  case CMD_INFO:
    systemInfo.logSystemInformation();
    break;
  case CMD_DEBUG:
    Log.infoln("Debug is %s now %s",
               preferencesStorage.debugEnabled ? "enabled" : "disabled",
               !preferencesStorage.debugEnabled ? "enabled" : "disabled");
    preferencesStorage.debugEnabled = !preferencesStorage.debugEnabled;
    break;
  case CMD_TTY0_TO_TTY1_BRIDGE:
    Log.infoln("TTY0 to TTY1 bridge is %s now %s",
               preferencesStorage.tty02tty1Bridge ? "enabled" : "disabled",
               !preferencesStorage.tty02tty1Bridge ? "enabled" : "disabled");
    preferencesStorage.tty02tty1Bridge = !preferencesStorage.tty02tty1Bridge;
    break;
  case CMD_RESET:
    for (int i = 5; i > 0; i--) {
      Log.infoln("%s: Resetting... %d any key to cancel", __PRETTY_FUNCTION__,
                 i);
      delay(1000);
      if (Serial.available() > 0) {
        Serial.read();
        Log.infoln("%s: Reset cancelled by user", __PRETTY_FUNCTION__);
        return false;
      }
    }
    Log.infoln("%s: Resetting see you later alligator :P...",
               __PRETTY_FUNCTION__);
    ESP.restart();
    break;
  default:
    Log.errorln("Unknown special character: %c", c);
    break;
  }
  return false;
}
} // namespace jrb::wifi_serial