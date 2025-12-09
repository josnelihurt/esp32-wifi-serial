#include "mqtt_handler_create_task.h"

namespace jrb::wifi_serial {

void MqttHandlerCreateTask::setup() {
    mqttHandler = new MqttClient(wifiClient);
    mqttHandler->setDeviceName(configManager.deviceName());
    mqttHandler->setTopics(configManager.topicTty0Rx(), configManager.topicTty0Tx(),
                          configManager.topicTty1Rx(), configManager.topicTty1Tx());
    mqttHandler->setCallbacks(onTty0, onTty1);
    
    Serial.print("[MQTT] Callbacks set - onTty0: ");
    Serial.println(onTty0 ? "SET" : "NULL");
    Serial.print("[MQTT] Callbacks set - onTty1: ");
    Serial.println(onTty1 ? "SET" : "NULL");
    
    if (configManager.mqttBroker().length() > 0) {
        const char* user = configManager.mqttUser().length() > 0 ? configManager.mqttUser().c_str() : nullptr;
        const char* pass = configManager.mqttPassword().length() > 0 ? configManager.mqttPassword().c_str() : nullptr;
        mqttHandler->connect(configManager.mqttBroker().c_str(), configManager.mqttPort(), user, pass);
    }
}

}  // namespace jrb::wifi_serial

