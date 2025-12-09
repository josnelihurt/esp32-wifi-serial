#include "system_info.h"

namespace jrb::wifi_serial {

void SystemInfo::printWelcomeMessage() {
    String macAddress = WiFi.macAddress();
    uint64_t chipId = ESP.getEfuseMac();
    char serialStr[13];
    snprintf(serialStr, sizeof(serialStr), "%04X%08X", (uint16_t)(chipId >> 32), (uint32_t)chipId);
    
    Serial.println("\n========================================");
    Serial.println("Welcome to ESP32-C3 Serial Bridge");
    Serial.println("========================================");
    Serial.print("Serial: ");
    Serial.println(serialStr);
    Serial.print("MAC: ");
    Serial.println(macAddress);
    Serial.print("Device Name: ");
    Serial.println(configManager.deviceName());
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("WiFi SSID: ");
        Serial.println(WiFi.SSID());
    } else {
        Serial.println("IP Address: Not connected");
        Serial.println("WiFi SSID: Not configured");
    }
    
    Serial.print("MQTT Broker: ");
    if (configManager.mqttBroker().length() > 0) {
        Serial.print(configManager.mqttBroker());
        Serial.print(":");
        Serial.println(configManager.mqttPort());
    } else {
        Serial.println("Not configured");
    }
    
    Serial.println("MQTT Topics:");
    Serial.print("  ttyS0 RX: ");
    Serial.println(configManager.topicTty0Rx());
    Serial.print("  ttyS0 TX: ");
    Serial.println(configManager.topicTty0Tx());
    Serial.print("  ttyS1 RX: ");
    Serial.println(configManager.topicTty1Rx());
    Serial.print("  ttyS1 TX: ");
    Serial.println(configManager.topicTty1Tx());
    
    Serial.print("MQTT Status: ");
    if (mqttHandler && mqttHandler->isConnected()) {
        Serial.println("Connected");
    } else {
        Serial.println("Disconnected");
    }
    
    Serial.print("Serial Bridge: ");
    Serial.println("Active");
    
    Serial.print("OTA: ");
    Serial.println(otaEnabled ? "Enabled" : "Disabled");
    
    Serial.println("========================================\n");
}

}  // namespace jrb::wifi_serial

