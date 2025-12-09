#pragma once

#include "preferences_storage.h"
#include "mqtt_client.h"
#include <WiFi.h>
#include <Arduino.h>

namespace jrb::wifi_serial {

class SystemInfo final {
private:
    PreferencesStorage& configManager;
    MqttClient* mqttHandler;
    bool& otaEnabled;

public:
    SystemInfo(PreferencesStorage& config, MqttClient* handler, bool& ota)
        : configManager(config), mqttHandler(handler), otaEnabled(ota) {}
    
    void printWelcomeMessage();
};

}  // namespace jrb::wifi_serial

