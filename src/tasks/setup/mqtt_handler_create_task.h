#pragma once

#include "interfaces/itask.h"
#include "mqtt_client.h"
#include "preferences_storage.h"
#include "dependency_container.h"
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
    DependencyContainer& container;
    WiFiClient& wifiClient;
    PreferencesStorage& preferencesStorage;
    std::function<void(const char*, unsigned int)> onTty0;
    std::function<void(const char*, unsigned int)> onTty1;

public:
    MqttHandlerCreateTask(DependencyContainer& cont, WiFiClient& wifi, 
                             PreferencesStorage& storage,
                             std::function<void(const char*, unsigned int)> tty0,
                             std::function<void(const char*, unsigned int)> tty1);
    
    void setup() override;
    void loop() override {}
    
    friend void tty0Wrapper(const char* data, unsigned int length);
    friend void tty1Wrapper(const char* data, unsigned int length);
};

}  // namespace jrb::wifi_serial

