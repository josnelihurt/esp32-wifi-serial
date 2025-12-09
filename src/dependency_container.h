#pragma once

#include "task_registry.h"
#include "wifi_manager.h"
#include "serial_bridge.h"
#include "serial_log.h"
#include "preferences_storage.h"
#include "mqtt_client.h"
#include "handlers/serial_command_handler.h"
#include "handlers/button_handler.h"
#include "system_info.h"
#include "ota_manager.h"
#include "web_config_server.h"
#include <WiFiClient.h>
#include <Preferences.h>
#include "config.h"

namespace jrb::wifi_serial {

class DependencyContainer final {
public:
    DependencyContainer();
    ~DependencyContainer();
    
    void createHandlers(std::function<void(int, const String&)> webToSerialCallback);
    
    TaskRegistry& getRegistry() { return registry; }
    WiFiManager& getWiFiManager() { return wifiManager; }
    MqttClient* getMqttClient() { return mqttClient; }
    void setMqttClient(MqttClient* client) { mqttClient = client; }
    PreferencesStorage& getPreferencesStorage() { return preferencesStorage; }
    SerialBridge& getSerialBridge() { return serialBridge; }
    SerialLog& getSerial0Log() { return serial0Log; }
    SerialLog& getSerial1Log() { return serial1Log; }
    bool& getDebugEnabled() { return debugEnabled; }
    unsigned long& getLastInfoPublish() { return lastInfoPublish; }
    char* getSerialBuffer(int portIndex) { 
        return (portIndex >= 0 && portIndex < 2) ? serialBuffer[portIndex] : nullptr;
    }
    Preferences& getPreferences() { return preferences; }
    WiFiClient& getWiFiClient() { return wifiClient; }
    SystemInfo* getSystemInfo() { return systemInfo; }
    ButtonHandler* getButtonHandler() { return buttonHandler; }
    OTAManager* getOTAManager() { return otaManager; }
    WebConfigServer* getWebConfigServer() { return webServer; }
    SerialCommandHandler* getSerialCommandHandler() { return serialCmdHandler; }
    
private:
    Preferences preferences;
    PreferencesStorage preferencesStorage;
    WiFiManager wifiManager;
    WiFiClient wifiClient;
    MqttClient* mqttClient{};
    SerialBridge serialBridge;
    SerialLog serial0Log;
    SerialLog serial1Log;
    
    bool otaEnabled{false};
    unsigned long lastInfoPublish{0};
    bool debugEnabled{false};
    char serialBuffer[2][SERIAL_BUFFER_SIZE];
    
    SystemInfo* systemInfo{};
    SerialCommandHandler* serialCmdHandler{};
    ButtonHandler* buttonHandler{};
    OTAManager* otaManager{};
    WebConfigServer* webServer{};
    
    TaskRegistry registry;
};

}  // namespace jrb::wifi_serial

