#include "special_character_handler.h"
#include "domain/config/preferences_storage.h"
#include "infrastructure/logging/logger.h"
#include "system_info.h"
namespace jrb::wifi_serial {
namespace internal {
template <typename ResetPolicy>
bool SpecialCharacterHandler<ResetPolicy>::handle(char c) {
  if (c == CMD_PREFIX) {
    specialCharacterMode = true;
    LOG_INFO("%s", systemInfo.getSpecialCharacterSettings().c_str());
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
    LOG_INFO("Debug is %s now %s",
             preferencesStorage.debugEnabled ? "enabled" : "disabled",
             !preferencesStorage.debugEnabled ? "enabled" : "disabled");
    preferencesStorage.debugEnabled = !preferencesStorage.debugEnabled;
    break;
  case CMD_TTY0_TO_TTY1_BRIDGE:
    LOG_INFO("TTY0 to TTY1 bridge is %s now %s",
             preferencesStorage.tty02tty1Bridge ? "enabled" : "disabled",
             !preferencesStorage.tty02tty1Bridge ? "enabled" : "disabled");
    preferencesStorage.tty02tty1Bridge = !preferencesStorage.tty02tty1Bridge;
    break;
  case CMD_RESET:
    for (int i = 5; i > 0; i--) {
      LOG_INFO("%s: Resetting... %d any key to cancel", __PRETTY_FUNCTION__, i);
      resetPolicy.delay(1000);
      if (resetPolicy.isSerialDataAvailable()) {
        resetPolicy.readSerialData();
        LOG_INFO("%s: Reset cancelled by user", __PRETTY_FUNCTION__);
        return false;
      }
    }
    LOG_INFO("%s: Resetting see you later alligator :P...",
             __PRETTY_FUNCTION__);
    resetPolicy.resetDevice();
    break;
  default:
    LOG_ERROR("Unknown special character: %c", c);
    break;
  }
  return false;
}
} // namespace internal

template class internal::SpecialCharacterHandler<HardwareResetPolicy>;

} // namespace jrb::wifi_serial
