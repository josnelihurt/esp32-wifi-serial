#pragma once

#include "domain/config/preferences_storage_policy.h"
#include <Arduino.h>
#include <WiFi.h>
#include <string>

namespace jrb::wifi_serial {

class SystemInfo final {
private:
  const PreferencesStorageDefault &preferencesStorage;
  bool &otaEnabled;

public:
  SystemInfo(const PreferencesStorageDefault &storage, bool &ota);

  std::string getSpecialCharacterSettings() const;
  std::string getWelcomeString() const;
  void logSystemInformation() const;
};

} // namespace jrb::wifi_serial
