#pragma once

#include "domain/config/preferences_storage_policy.h"
#include "infrastructure/types.hpp"

namespace jrb::wifi_serial {

class SystemInfo final {
private:
  const PreferencesStorage &preferencesStorage;
  bool &otaEnabled;

public:
  SystemInfo(const PreferencesStorage &storage, bool &ota);

  types::string getSpecialCharacterSettings() const;
  types::string getWelcomeString() const;
  void logSystemInformation() const;
};

} // namespace jrb::wifi_serial
