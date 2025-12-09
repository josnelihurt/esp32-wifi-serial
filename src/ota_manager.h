#pragma once

#include "interfaces/iota_manager.h"
#include "domain/config/preferences_storage.h"
#include <ArduinoOTA.h>

namespace jrb::wifi_serial {

class OTAManager final : public IOTAManager {
private:
    PreferencesStorage& preferencesStorage;
    bool& otaEnabled;

public:
    OTAManager(PreferencesStorage& storage, bool& ota)
        : preferencesStorage(storage), otaEnabled(ota) {}
    
    void setup() override;
};

}  // namespace jrb::wifi_serial

