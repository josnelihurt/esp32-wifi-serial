#pragma once

#include "interfaces/itask.h"
#include "infrastructure/web/web_config_server.h"
#include "domain/network/wifi_manager.h"

namespace jrb::wifi_serial {

class WebConfigServerSetupTask final : public ITask {
private:
    WebConfigServer& webServer;
    WiFiManager& wifiManager;

public:
    WebConfigServerSetupTask(WebConfigServer& server, WiFiManager& wifi)
        : webServer(server), wifiManager(wifi) {}
    
    void setup() override {
        webServer.setWiFiConfig(
            wifiManager.getSSID(),
            wifiManager.getPassword(),
            wifiManager.getDeviceName(),
            wifiManager.getMqttBroker(),
            wifiManager.getMqttPort(),
            wifiManager.getMqttUser(),
            wifiManager.getMqttPassword()
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

