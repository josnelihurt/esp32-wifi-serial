#include "mqtt_info_publish_task.h"

namespace jrb::wifi_serial {

void MqttInfoPublishTask::loop() {
    if (!mqttHandler) return;
    if (!mqttHandler->isConnected()) return;
    if (millis() - lastInfoPublish < INFO_PUBLISH_INTERVAL) return;
    
    String macAddress = WiFi.macAddress();
    String ipAddress = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "Not connected";
    String ssid = WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "Not configured";
    
    String infoJson = "{\"device\":\"" + configManager.deviceName() + "\",\"ip\":\"" + ipAddress + 
                     "\",\"mac\":\"" + macAddress + "\",\"ssid\":\"" + ssid + 
                     "\",\"mqtt\":\"" + (configManager.mqttBroker().length() > 0 ? "connected" : "disconnected") + "\"}";
    mqttHandler->publishInfo(infoJson);
    lastInfoPublish = millis();
}

}  // namespace jrb::wifi_serial

