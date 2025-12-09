#include "mqtt_reconnect_task.h"

namespace jrb::wifi_serial {

void MqttReconnectTask::loop() {
    if (!mqttHandler) return;
    if (configManager.mqttBroker().length() == 0) return;
    if (mqttHandler->isConnected()) return;
    
    static unsigned long lastReconnectAttempt = 0;
    unsigned long now = millis();
    
    if (now - lastReconnectAttempt < 5000) {
        return;
    }
    lastReconnectAttempt = now;
    
    Serial.println("[MQTT] Attempting to reconnect...");
    const char* user = configManager.mqttUser().length() > 0 ? configManager.mqttUser().c_str() : nullptr;
    const char* pass = configManager.mqttPassword().length() > 0 ? configManager.mqttPassword().c_str() : nullptr;
    
    bool result = mqttHandler->connect(configManager.mqttBroker().c_str(), configManager.mqttPort(), user, pass);
    if (!result) {
        Serial.print("[MQTT] Reconnect failed, state: ");
        Serial.println(mqttHandler->getMqttClient() ? mqttHandler->getMqttClient()->state() : -1);
        return;
    } 
    Serial.println("[MQTT] Reconnected successfully!");
}

}  // namespace jrb::wifi_serial

