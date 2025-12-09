#pragma once

#include "preferences_storage.h"
#include <ArduinoOTA.h>

namespace jrb::wifi_serial {

class OTAManager final {
private:
    PreferencesStorage& configManager;
    bool& otaEnabled;

public:
    OTAManager(PreferencesStorage& config, bool& ota)
        : configManager(config), otaEnabled(ota) {}
    
    void setup();
};

}  // namespace jrb::wifi_serial

