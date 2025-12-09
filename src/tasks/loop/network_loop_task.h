#pragma once

#include "interfaces/itask.h"
#include "wifi_manager.h"
#include "mqtt_client.h"
#include "dependency_container.h"
#include <ArduinoOTA.h>

namespace jrb::wifi_serial {

class NetworkLoopTask final : public ITask {
private:
    WiFiManager& wifiManager;

public:
    explicit NetworkLoopTask(WiFiManager& wifi)
        : wifiManager(wifi) {}
    
    void loop() override {
        wifiManager.loop();
        
        if (DependencyContainer::instance) {
            MqttClient* mqttClient = DependencyContainer::instance->getMqttClient();
            if (mqttClient) {
                mqttClient->loop();
            }
        }
        
        ArduinoOTA.handle();
    }
};

}  // namespace jrb::wifi_serial

