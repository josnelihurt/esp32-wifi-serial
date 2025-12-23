#pragma once

#include "domain/config/preferences_storage_policy.h"

namespace jrb::wifi_serial {

class OTAManager final {
private:
  PreferencesStorage &preferencesStorage;
  bool &otaEnabled;

public:
  OTAManager(PreferencesStorage &storage, bool &ota)
      : preferencesStorage(storage), otaEnabled(ota) {}

  void setup();
};

} // namespace jrb::wifi_serial
