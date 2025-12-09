#pragma once

#include "interfaces/itask.h"
#include "config.h"
#include "wifi_manager.h"
#include "preferences_storage.h"

namespace jrb::wifi_serial {

class ConfigManagerSyncTask final : public ITask {
private:
    WiFiManager& wifiConfig;
    PreferencesStorage& configManager;

public:
    ConfigManagerSyncTask(WiFiManager& wifi, PreferencesStorage& config)
        : wifiConfig(wifi), configManager(config) {}
    
    void setup() override;
    void loop() override {}
};

}  // namespace jrb::wifi_serial

