#pragma once

#include "interfaces/itask.h"
#include "mqtt_client.h"
#include "preferences_storage.h"
#include <WiFiClient.h>

namespace jrb::wifi_serial {

class MqttHandlerCreateTask final : public ITask {
private:
    MqttClient*& mqttClient;
    WiFiClient& wifiClient;
    PreferencesStorage& preferencesStorage;
    void (*onTty0)(const char*, unsigned int);
    void (*onTty1)(const char*, unsigned int);

public:
    MqttHandlerCreateTask(MqttClient*& client, WiFiClient& wifi, 
                             PreferencesStorage& storage,
                             void (*tty0)(const char*, unsigned int),
                             void (*tty1)(const char*, unsigned int))
        : mqttClient(client), wifiClient(wifi), preferencesStorage(storage),
          onTty0(tty0), onTty1(tty1) {}
    
    void setup() override;
    void loop() override {}
};

}  // namespace jrb::wifi_serial

