#pragma once

#include "domain/config/preferences_storage_policy.h"
#include "infrastructure/types.hpp"
#include <Arduino.h>
#include <WiFi.h>

namespace jrb::wifi_serial {

class SystemInfo final {
private:
  const PreferencesStorageDefault &preferencesStorage;
  bool &otaEnabled;

public:
  SystemInfo(const PreferencesStorageDefault &storage, bool &ota);

  types::string getSpecialCharacterSettings() const;
  types::string getWelcomeString() const;
  void logSystemInformation() const;
};

} // namespace jrb::wifi_serial
