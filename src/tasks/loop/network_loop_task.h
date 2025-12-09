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
    DependencyContainer& container;

public:
    NetworkLoopTask(WiFiManager& wifi, DependencyContainer& cont)
        : wifiManager(wifi), container(cont) {}
    
    void loop() override {
        wifiManager.loop();
        
        // IMPORTANT: Get MqttClient from container each loop() call, not stored as pointer.
        // This is critical because MqttClient is created AFTER this task is registered
        // (in MqttHandlerCreateTask::setup()). If we stored MqttClient* in constructor,
        // it would be nullptr and never updated, causing MQTT messages to never be processed.
        MqttClient* mqttClient = container.getMqttClient();
        if (mqttClient) {
            mqttClient->loop();
        }
        
        ArduinoOTA.handle();
    }
};

}  // namespace jrb::wifi_serial

