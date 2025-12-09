#include "mqtt_handler_create_task.h"

namespace jrb::wifi_serial {

void MqttHandlerCreateTask::setup() {
    mqttClient = new MqttClient(wifiClient);
    mqttClient->setDeviceName(preferencesStorage.deviceName());
    mqttClient->setTopics(preferencesStorage.topicTty0Rx(), preferencesStorage.topicTty0Tx(),
                          preferencesStorage.topicTty1Rx(), preferencesStorage.topicTty1Tx());
    mqttClient->setCallbacks(onTty0, onTty1);
    
    Serial.print("[MQTT] Callbacks set - onTty0: ");
    Serial.println(onTty0 ? "SET" : "NULL");
    Serial.print("[MQTT] Callbacks set - onTty1: ");
    Serial.println(onTty1 ? "SET" : "NULL");
    
    if (preferencesStorage.mqttBroker().length() > 0) {
        const char* user = preferencesStorage.mqttUser().length() > 0 ? preferencesStorage.mqttUser().c_str() : nullptr;
        const char* pass = preferencesStorage.mqttPassword().length() > 0 ? preferencesStorage.mqttPassword().c_str() : nullptr;
        mqttClient->connect(preferencesStorage.mqttBroker().c_str(), preferencesStorage.mqttPort(), user, pass);
    }
}

}  // namespace jrb::wifi_serial

