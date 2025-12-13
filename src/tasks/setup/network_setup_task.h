#pragma once

#include "interfaces/itask.h"
#include "domain/network/wifi_manager.h"
#include "ota_manager.h"
#include "system_info.h"
#include <Preferences.h>

namespace jrb::wifi_serial {

class NetworkSetupTask final : public ITask {
private:
    WiFiManager& wifiManager;
    ::Preferences& preferences;
    OTAManager& otaManager;
    SystemInfo& systemInfo;

public:
    NetworkSetupTask(WiFiManager& wifi, ::Preferences& prefs, OTAManager& ota, SystemInfo& info)
        : wifiManager(wifi), preferences(prefs), otaManager(ota), systemInfo(info) {}
    
    void setup() override {
        wifiManager.setup();
        otaManager.setup();
        systemInfo.logSystemInformation();
    }
    
    void loop() override {}
};

}  // namespace jrb::wifi_serial

