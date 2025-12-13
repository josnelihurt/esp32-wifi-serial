#pragma once

#include "interfaces/itask.h"
#include "domain/network/wifi_manager.h"
#include "interfaces/imqtt_client.h"
#include "dependency_container.h"
#include <ArduinoOTA.h>

namespace jrb::wifi_serial {

class NetworkLoopTask final : public ITask {
private:
    WiFiManager& wifiManager;
    DependencyContainer& container;

public:
    NetworkLoopTask(WiFiManager& wifi, DependencyContainer& cont)
        : wifiManager(wifi), container(cont) {}
    
    void loop() override {
        wifiManager.loop();
        container.getMqttClient().loop();
        ArduinoOTA.handle();
    }
};

}  // namespace jrb::wifi_serial

