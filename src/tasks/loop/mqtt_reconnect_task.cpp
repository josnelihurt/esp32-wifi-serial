#include "mqtt_reconnect_task.h"

namespace jrb::wifi_serial {

void MqttReconnectTask::loop() {
    if (!mqttClient) return;
    if (preferencesStorage.mqttBroker().length() == 0) return;
    if (mqttClient->isConnected()) return;
    
    static unsigned long lastReconnectAttempt = 0;
    unsigned long now = millis();
    
    if (now - lastReconnectAttempt < 5000) {
        return;
    }
    lastReconnectAttempt = now;
    
    Serial.println("[MQTT] Attempting to reconnect...");
    const char* user = preferencesStorage.mqttUser().length() > 0 ? preferencesStorage.mqttUser().c_str() : nullptr;
    const char* pass = preferencesStorage.mqttPassword().length() > 0 ? preferencesStorage.mqttPassword().c_str() : nullptr;
    
    bool result = mqttClient->connect(preferencesStorage.mqttBroker().c_str(), preferencesStorage.mqttPort(), user, pass);
    if (!result) {
        Serial.println("[MQTT] Reconnect failed");
        return;
    } 
    Serial.println("[MQTT] Reconnected successfully!");
}

}  // namespace jrb::wifi_serial

