#pragma once

#include "../../interfaces/itask.h"
#include "../../domain/network/mqtt_client.h"
#include "../../domain/config/preferences_storage.h"
#include <WiFiClient.h>
#include <functional>

namespace jrb::wifi_serial {

class MqttHandlerCreateTask;

// C-style callback wrappers for PubSubClient (requires C function pointers).
// These wrappers access g_taskInstance to call the std::function callbacks stored in the task.
void tty0Wrapper(const char* data, unsigned int length);
void tty1Wrapper(const char* data, unsigned int length);

class MqttHandlerCreateTask final : public ITask {
private:
    MqttClient& mqttClient;
    WiFiClient& wifiClient;
    PreferencesStorage& preferencesStorage;
    std::function<void(const char*, unsigned int)> onTty0;
    std::function<void(const char*, unsigned int)> onTty1;

public:
    MqttHandlerCreateTask(MqttClient& mqtt, WiFiClient& wifi,
                             PreferencesStorage& storage,
                             std::function<void(const char*, unsigned int)> tty0,
                             std::function<void(const char*, unsigned int)> tty1);
    
    void setup() override;
    void loop() override {}
    
    friend void tty0Wrapper(const char* data, unsigned int length);
    friend void tty1Wrapper(const char* data, unsigned int length);
};

}  // namespace jrb::wifi_serial

