#pragma once

#include "domain/config/preferences_storage.h"
#include "interfaces/imqtt_client.h"
#include <WiFi.h>
#include <Arduino.h>

namespace jrb::wifi_serial {

class SystemInfo final {
private:
    PreferencesStorage& preferencesStorage;
    IMqttClient* mqttClient;
    bool& otaEnabled;

public:
    SystemInfo(PreferencesStorage& storage, IMqttClient* client, bool& ota)
        : preferencesStorage(storage), mqttClient(client), otaEnabled(ota) {}
    
    void logSystemInformation() const;
};

}  // namespace jrb::wifi_serial

