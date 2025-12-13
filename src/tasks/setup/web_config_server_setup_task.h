#pragma once

#include "interfaces/itask.h"
#include "infrastructure/web/web_config_server.h"
#include "domain/network/wifi_manager.h"

namespace jrb::wifi_serial {

class WebConfigServerSetupTask final : public ITask {
private:
    WebConfigServer& webServer;
    WiFiManager& wifiManager;
    PreferencesStorage& preferencesStorage;
public:
    WebConfigServerSetupTask(WebConfigServer& server, WiFiManager& wifi, PreferencesStorage& preferencesStorage)
        : webServer(server), wifiManager(wifi), preferencesStorage(preferencesStorage) {}
    
    void setup() override {
        webServer.setWiFiConfig(
            preferencesStorage.ssid,
            preferencesStorage.password,
            preferencesStorage.deviceName,
            preferencesStorage.mqttBroker,
            preferencesStorage.mqttPort,
            preferencesStorage.mqttUser,
            preferencesStorage.mqttPassword
        );
        webServer.setAPMode(wifiManager.isAPMode());
        if (wifiManager.isAPMode()) {
            webServer.setAPIP(wifiManager.getAPIP());
        }
        webServer.begin();
    }
    void loop() override {}
};

}  // namespace jrb::wifi_serial

