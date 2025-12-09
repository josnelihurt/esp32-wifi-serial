#include "mqtt_handler_create_task.h"

namespace jrb::wifi_serial {

// Static pointer to task instance for C-style callback wrappers.
// PubSubClient requires C-style function pointers, so we use wrappers that
// access the task instance to call the std::function callbacks.
static MqttHandlerCreateTask* g_taskInstance = nullptr;

void tty0Wrapper(const char* data, unsigned int length) {
    if (g_taskInstance && g_taskInstance->onTty0) {
        g_taskInstance->onTty0(data, length);
    }
}

void tty1Wrapper(const char* data, unsigned int length) {
    if (g_taskInstance && g_taskInstance->onTty1) {
        g_taskInstance->onTty1(data, length);
    }
}

MqttHandlerCreateTask::MqttHandlerCreateTask(DependencyContainer& cont, WiFiClient& wifi, 
                                             PreferencesStorage& storage,
                                             std::function<void(const char*, unsigned int)> tty0,
                                             std::function<void(const char*, unsigned int)> tty1)
    : container(cont), wifiClient(wifi), preferencesStorage(storage),
      onTty0(tty0), onTty1(tty1) {
    g_taskInstance = this;
}

void MqttHandlerCreateTask::setup() {
    MqttClient* client = new MqttClient(wifiClient);
    container.setMqttClient(client);
    
    client->setDeviceName(preferencesStorage.deviceName());
    client->setTopics(preferencesStorage.topicTty0Rx(), preferencesStorage.topicTty0Tx(),
                      preferencesStorage.topicTty1Rx(), preferencesStorage.topicTty1Tx());
    
    client->setCallbacks(tty0Wrapper, tty1Wrapper);
    
    if (preferencesStorage.mqttBroker().length() > 0) {
        const char* user = preferencesStorage.mqttUser().length() > 0 ? preferencesStorage.mqttUser().c_str() : nullptr;
        const char* pass = preferencesStorage.mqttPassword().length() > 0 ? preferencesStorage.mqttPassword().c_str() : nullptr;
        client->connect(preferencesStorage.mqttBroker().c_str(), preferencesStorage.mqttPort(), user, pass);
    }
}

}  // namespace jrb::wifi_serial

