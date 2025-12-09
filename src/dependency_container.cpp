#include "dependency_container.h"
#include "tasks/setup/hardware_setup_task.h"
#include "tasks/setup/config_setup_task.h"
#include "tasks/setup/triple_press_check_setup_task.h"
#include "tasks/setup/network_setup_task.h"
#include "tasks/setup/wifi_config_ap_mode_wait_task.h"
#include "tasks/setup/config_manager_sync_task.h"
#include "tasks/setup/save_config_task.h"
#include "tasks/setup/mqtt_handler_create_task.h"
#include "tasks/setup/web_config_server_setup_task.h"
#include "tasks/loop/network_loop_task.h"
#include "tasks/loop/system_loop_task.h"
#include "tasks/loop/triple_press_check_task.h"
#include "tasks/loop/mqtt_reconnect_task.h"
#include "tasks/loop/mqtt_info_publish_task.h"
#include "tasks/loop/serial_bridge_task.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

DependencyContainer* DependencyContainer::instance{};

static void onTty0Wrapper(const char* data, unsigned int length) {
    if (DependencyContainer::instance) {
        DependencyContainer::instance->onTty0(data, length);
    }
}

static void onTty1Wrapper(const char* data, unsigned int length) {
    if (DependencyContainer::instance) {
        DependencyContainer::instance->onTty1(data, length);
    }
}

static void resetConfigWrapper() {
    if (DependencyContainer::instance) {
        DependencyContainer::instance->resetConfig();
    }
}

DependencyContainer::DependencyContainer() {
    instance = this;
    createHandlers();
    serialBridge.setLogs(serial0Log, serial1Log);
}

DependencyContainer::~DependencyContainer() {
    if (instance == this) {
        instance = nullptr;
    }
    delete systemInfo;
    delete serialCmdHandler;
    delete buttonHandler;
    delete otaManager;
}

void DependencyContainer::onTty0(const char* data, unsigned int length) {
    serialBridge.handleMqttToSerialAndWeb(0, data, length);
}

void DependencyContainer::onTty1(const char* data, unsigned int length) {
    serialBridge.handleMqttToSerialAndWeb(1, data, length);
}

void DependencyContainer::handleWebToSerialAndMqtt(int portIndex, const String& data) {
    serialBridge.handleWebToSerialAndMqtt(portIndex, data);
}

void DependencyContainer::resetConfig() {
    preferencesStorage.clear();
    wifiManager.resetSettings();
    configResetRequested = true;
}

void DependencyContainer::createHandlers() {
    systemInfo = new SystemInfo(preferencesStorage, mqttClient, otaEnabled);
    serialCmdHandler = new SerialCommandHandler(
        preferencesStorage, mqttClient, debugEnabled,
        [this]() { systemInfo->printWelcomeMessage(); }
    );
    buttonHandler = new ButtonHandler(
        [this]() { systemInfo->printWelcomeMessage(); }
    );
    otaManager = new OTAManager(preferencesStorage, otaEnabled);
    webServer = new WebConfigServer(
        preferencesStorage, serial0Log, serial1Log,
        [this](int portIndex, const String& data) {
            handleWebToSerialAndMqtt(portIndex, data);
        }
    );
}

void DependencyContainer::registerSetupTasks() {
    static HardwareSetupTask hardwareSetupCmd(serialBridge);
    static ConfigSetupTask configSetupCmd(preferencesStorage, *systemInfo);
    
    static TriplePressCheckSetupTask triplePressCheckCmd(*buttonHandler, resetConfigWrapper);
    
    static NetworkSetupTask networkSetupCmd(wifiManager, preferences, *otaManager, *systemInfo);
    static WiFiConfigAPModeWaitTask wifiAPWaitCmd(wifiManager);
    static ConfigManagerSyncTask configSyncCmd(wifiManager, preferencesStorage);
    static SaveConfigTask saveConfigCmd(preferencesStorage);
    
    static MqttHandlerCreateTask mqttCreateCmd(mqttClient, wifiClient, 
                                                  preferencesStorage, 
                                                  onTty0Wrapper,
                                                  onTty1Wrapper);
    static WebConfigServerSetupTask webServerSetupCmd(*webServer, wifiManager);
    
    registry.registerTask(&hardwareSetupCmd);
    registry.registerTask(&configSetupCmd);
    registry.registerTask(&triplePressCheckCmd);
    registry.registerTask(&networkSetupCmd);
    registry.registerTask(&wifiAPWaitCmd);
    registry.registerTask(&configSyncCmd);
    registry.registerTask(&saveConfigCmd);
    registry.registerTask(&mqttCreateCmd);
    registry.registerTask(&webServerSetupCmd);
}

void DependencyContainer::registerLoopTasks() {
    static MqttReconnectTask mqttReconnectCmd(mqttClient, preferencesStorage);
    static NetworkLoopTask networkLoopCmd(wifiManager);
    static SystemLoopTask systemLoopCmd(serialCmdHandler);
    
    static TriplePressCheckTask triplePressCmd(buttonHandler, resetConfigWrapper);
    
    static MqttInfoPublishTask mqttInfoCmd(mqttClient, preferencesStorage, lastInfoPublish);
    static SerialBridge0Task serial0Cmd(serialBridge, serial0Log, serialBuffer[0], 
                                           mqttClient, debugEnabled);
    static SerialBridge1Task serial1Cmd(serialBridge, serial1Log, serialBuffer[1], 
                                           mqttClient, debugEnabled);
    
    registry.registerTask(&mqttReconnectCmd);
    registry.registerTask(&networkLoopCmd);
    registry.registerTask(&systemLoopCmd);
    registry.registerTask(&triplePressCmd);
    registry.registerTask(&mqttInfoCmd);
    registry.registerTask(&serial0Cmd);
    registry.registerTask(&serial1Cmd);
}

void DependencyContainer::initialize() {
    registerSetupTasks();
    registerLoopTasks();
}

void DependencyContainer::setup() {
    registry.setupAll();
    serialBridge.setMqttHandler(mqttClient);
    Serial.println("Setup complete!");
}

void DependencyContainer::loop() {
    registry.loopAll();
    
    if (webServer) {
        webServer->loop();
    }
    
    if (mqttClient) {
        for (int i = 0; i < 2; i++) {
            if (mqttClient->shouldFlushBuffer(i)) {
                mqttClient->flushBuffer(i);
            }
        }
    }
}

}  // namespace jrb::wifi_serial

