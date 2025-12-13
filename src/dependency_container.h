#pragma once

#include "task_registry.h"
#include "domain/network/wifi_manager.h"
#include "domain/serial/serial_bridge.h"
#include "domain/serial/serial_log.h"
#include "domain/config/preferences_storage.h"
#include "domain/network/mqtt_client.h"
#include "infrastructure/hardware/serial_command_handler.h"
#include "infrastructure/hardware/button_handler.h"
#include "system_info.h"
#include "ota_manager.h"
#include "infrastructure/web/web_config_server.h"
#include "interfaces/imqtt_client.h"
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
    IMqttClient& getMqttClient() { return mqttClient; }
    PreferencesStorage& getPreferencesStorage() { return preferencesStorage; }
    SerialBridge& getSerialBridge() { return serialBridge; }
    SerialLog& getSerial0Log() { return serial0Log; }
    SerialLog& getSerial1Log() { return serial1Log; }
    bool& getDebugEnabled() { return debugEnabled; }
    unsigned long& getLastInfoPublish() { return lastInfoPublish; }
    char* getSerialBuffer(int portIndex) { 
        return (portIndex >= 0 && portIndex < 2) ? serialBuffer[portIndex] : nullptr;
    }
    ::Preferences& getPreferences() { return preferences; }
    WiFiClient& getWiFiClient() { return wifiClient; }
    SystemInfo& getSystemInfo() { return systemInfo; }
    ButtonHandler* getButtonHandler() { return buttonHandler; }
    OTAManager* getOTAManager() { return otaManager; }
    WebConfigServer* getWebConfigServer() { return webServer; }
    SerialCommandHandler* getSerialCommandHandler() { return serialCmdHandler; }
    
private:
    ::Preferences preferences;
    PreferencesStorage preferencesStorage;
    WiFiManager wifiManager{preferencesStorage};
    WiFiClient wifiClient;
    MqttClient mqttClient{wifiClient};
    SerialBridge serialBridge;
    SerialLog serial0Log;
    SerialLog serial1Log;
    
    bool otaEnabled{false};
    unsigned long lastInfoPublish{0};
    bool debugEnabled{true};
    char serialBuffer[2][SERIAL_BUFFER_SIZE];
    
    SystemInfo systemInfo{preferencesStorage, &mqttClient, otaEnabled};
    SerialCommandHandler* serialCmdHandler{};
    ButtonHandler* buttonHandler{};
    OTAManager* otaManager{};
    WebConfigServer* webServer{};
    
    TaskRegistry registry;
};

}  // namespace jrb::wifi_serial

