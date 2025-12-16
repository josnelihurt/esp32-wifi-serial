#pragma once
#include "domain/config/preferences_storage.h"
#include "system_info.h"

namespace jrb::wifi_serial {

class SpecialCharacterHandler {
private:
  bool specialCharacterMode;
  SystemInfo &systemInfo;
  PreferencesStorage &preferencesStorage;

public:
  SpecialCharacterHandler(SystemInfo &systemInfo,
                          PreferencesStorage &preferencesStorage);
  bool handle(char c);
};
} // namespace jrb::wifi_serial