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
    
    void initialize();
    void setup();
    void loop();
    
    void onTty0(const char* data, unsigned int length);
    void onTty1(const char* data, unsigned int length);
    void resetConfig();
    
    void handleWebToSerialAndMqtt(int portIndex, const String& data);
    
    TaskRegistry& getRegistry() { return registry; }
    WiFiManager& getWiFiConfig() { return wifiConfig; }
    MqttClient* getMqttHandler() { return mqttHandler; }
    PreferencesStorage& getConfigManager() { return configManager; }
    SerialBridge& getSerialBridge() { return serialBridge; }
    SerialLog& getSerial0Log() { return serial0Log; }
    SerialLog& getSerial1Log() { return serial1Log; }
    bool& getDebugEnabled() { return debugEnabled; }
    unsigned long& getLastInfoPublish() { return lastInfoPublish; }
    char* getSerialBuffer(int portIndex) { 
        return (portIndex >= 0 && portIndex < 2) ? serialBuffer[portIndex] : nullptr;
    }
    
    static DependencyContainer* instance;

private:
    Preferences preferences;
    PreferencesStorage configManager;
    WiFiManager wifiConfig;
    WiFiClient wifiClient;
    MqttClient* mqttHandler{};
    SerialBridge serialBridge;
    SerialLog serial0Log;
    SerialLog serial1Log;
    
    bool otaEnabled{false};
    unsigned long lastInfoPublish{0};
    bool configResetRequested{false};
    bool debugEnabled{false};
    char serialBuffer[2][SERIAL_BUFFER_SIZE];
    
    SystemInfo* systemInfo{};
    SerialCommandHandler* serialCmdHandler{};
    ButtonHandler* buttonHandler{};
    OTAManager* otaManager{};
    WebConfigServer* webServer{};
    
    TaskRegistry registry;
    
    void createHandlers();
    void registerSetupTasks();
    void registerLoopTasks();
};

}  // namespace jrb::wifi_serial

