#pragma once

#include "interfaces/itask.h"
#include "config.h"
#include "wifi_manager.h"
#include "preferences_storage.h"

namespace jrb::wifi_serial {

class ConfigManagerSyncTask final : public ITask {
private:
    WiFiManager& wifiManager;
    PreferencesStorage& preferencesStorage;

public:
    ConfigManagerSyncTask(WiFiManager& wifi, PreferencesStorage& storage)
        : wifiManager(wifi), preferencesStorage(storage) {}
    
    void setup() override;
    void loop() override {}
};

}  // namespace jrb::wifi_serial

