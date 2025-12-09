#pragma once

#include "interfaces/itask.h"
#include "mqtt_client.h"
#include "preferences_storage.h"
#include <WiFiClient.h>

namespace jrb::wifi_serial {

class MqttHandlerCreateTask final : public ITask {
private:
    MqttClient*& mqttHandler;
    WiFiClient& wifiClient;
    PreferencesStorage& configManager;
    void (*onTty0)(const char*, unsigned int);
    void (*onTty1)(const char*, unsigned int);

public:
    MqttHandlerCreateTask(MqttClient*& handler, WiFiClient& client, 
                             PreferencesStorage& config,
                             void (*tty0)(const char*, unsigned int),
                             void (*tty1)(const char*, unsigned int))
        : mqttHandler(handler), wifiClient(client), configManager(config),
          onTty0(tty0), onTty1(tty1) {}
    
    void setup() override;
    void loop() override {}
};

}  // namespace jrb::wifi_serial

