#pragma once

#include "interfaces/itask.h"
#include "wifi_manager.h"
#include <Preferences.h>

namespace jrb::wifi_serial {

class WiFiConfigBeginTask final : public ITask {
private:
    WiFiManager& wifiConfig;
    Preferences& preferences;

public:
    WiFiConfigBeginTask(WiFiManager& config, Preferences& prefs)
        : wifiConfig(config), preferences(prefs) {}
    
    void setup() override {
        wifiConfig.begin(&preferences);
    }
    void loop() override {}
};

}  // namespace jrb::wifi_serial

