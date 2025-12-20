#pragma once

#include "domain/config/preferences_storage_policy.h"
#include <ArduinoOTA.h>

namespace jrb::wifi_serial {

class OTAManager final {
private:
  PreferencesStorageDefault &preferencesStorage;
  bool &otaEnabled;

public:
  OTAManager(PreferencesStorageDefault &storage, bool &ota)
      : preferencesStorage(storage), otaEnabled(ota) {}

  void setup();
};

} // namespace jrb::wifi_serial
