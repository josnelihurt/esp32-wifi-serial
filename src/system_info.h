#pragma once

#include "preferences_storage.h"
#include "mqtt_client.h"
#include <WiFi.h>
#include <Arduino.h>

namespace jrb::wifi_serial {

class SystemInfo final {
private:
    PreferencesStorage& preferencesStorage;
    MqttClient* mqttClient;
    bool& otaEnabled;

public:
    SystemInfo(PreferencesStorage& storage, MqttClient* client, bool& ota)
        : preferencesStorage(storage), mqttClient(client), otaEnabled(ota) {}
    
    void printWelcomeMessage();
};

}  // namespace jrb::wifi_serial

