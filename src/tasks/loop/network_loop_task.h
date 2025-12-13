#pragma once

#include "../../interfaces/itask.h"
#include "../../domain/network/wifi_manager.h"
#include "../../interfaces/imqtt_client.h"
#include <ArduinoOTA.h>

namespace jrb::wifi_serial {

class NetworkLoopTask final : public ITask {
private:
    WiFiManager& wifiManager;
    IMqttClient& mqttClient;

public:
    NetworkLoopTask(WiFiManager& wifi, IMqttClient& mqtt)
        : wifiManager(wifi), mqttClient(mqtt) {}

    void loop() override {
        wifiManager.loop();
        mqttClient.loop();
        ArduinoOTA.handle();
    }
};

}  // namespace jrb::wifi_serial

