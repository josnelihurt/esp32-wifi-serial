#include "dependency_container.h"
#include "tasks/setup/pin_mode_setup_task.h"
#include "tasks/setup/serial_begin_task.h"
#include "tasks/setup/load_config_task.h"
#include "tasks/setup/print_welcome_task.h"
#include "tasks/setup/triple_press_check_setup_task.h"
#include "tasks/setup/wifi_config_begin_task.h"
#include "tasks/setup/wifi_config_ap_mode_wait_task.h"
#include "tasks/setup/config_manager_sync_task.h"
#include "tasks/setup/save_config_task.h"
#include "tasks/setup/ota_setup_task.h"
#include "tasks/setup/mqtt_handler_create_task.h"
#include "tasks/setup/serial_bridge_begin_task.h"
#include "tasks/setup/web_config_server_setup_task.h"
#include "tasks/loop/wifi_config_task.h"
#include "tasks/loop/arduino_ota_task.h"
#include "tasks/loop/serial_commands_task.h"
#include "tasks/loop/triple_press_check_task.h"
#include "tasks/loop/mqtt_handler_task.h"
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
    configManager.clear();
    wifiConfig.resetSettings();
    configResetRequested = true;
}

void DependencyContainer::createHandlers() {
    systemInfo = new SystemInfo(configManager, mqttHandler, otaEnabled);
    serialCmdHandler = new SerialCommandHandler(
        configManager, mqttHandler, debugEnabled,
        [this]() { systemInfo->printWelcomeMessage(); }
    );
    buttonHandler = new ButtonHandler(
        [this]() { systemInfo->printWelcomeMessage(); }
    );
    otaManager = new OTAManager(configManager, otaEnabled);
    webServer = new WebConfigServer(
        configManager, serial0Log, serial1Log,
        [this](int portIndex, const String& data) {
            handleWebToSerialAndMqtt(portIndex, data);
        }
    );
}

void DependencyContainer::registerSetupTasks() {
    static PinModeSetupTask pinModeCmd;
    static SerialBeginTask serialBeginCmd;
    static LoadConfigTask loadConfigCmd(configManager);
    static PrintWelcomeTask printWelcomeCmd1(*systemInfo);
    
    static TriplePressCheckSetupTask triplePressCheckCmd(*buttonHandler, resetConfigWrapper);
    
    static WiFiConfigBeginTask wifiBeginCmd(wifiConfig, preferences);
    static WiFiConfigAPModeWaitTask wifiAPWaitCmd(wifiConfig);
    static ConfigManagerSyncTask configSyncCmd(wifiConfig, configManager);
    static SaveConfigTask saveConfigCmd(configManager);
    static PrintWelcomeTask printWelcomeCmd2(*systemInfo);
    static OTASetupTask otaSetupCmd(*otaManager);
    
    static MqttHandlerCreateTask mqttCreateCmd(mqttHandler, wifiClient, 
                                                  configManager, 
                                                  onTty0Wrapper,
                                                  onTty1Wrapper);
    static SerialBridgeBeginTask serialBridgeBeginCmd(serialBridge);
    static WebConfigServerSetupTask webServerSetupCmd(*webServer, wifiConfig);
    
    registry.registerTask(&pinModeCmd);
    registry.registerTask(&serialBeginCmd);
    registry.registerTask(&loadConfigCmd);
    registry.registerTask(&printWelcomeCmd1);
    registry.registerTask(&triplePressCheckCmd);
    registry.registerTask(&wifiBeginCmd);
    registry.registerTask(&wifiAPWaitCmd);
    registry.registerTask(&configSyncCmd);
    registry.registerTask(&saveConfigCmd);
    registry.registerTask(&printWelcomeCmd2);
    registry.registerTask(&otaSetupCmd);
    registry.registerTask(&mqttCreateCmd);
    registry.registerTask(&serialBridgeBeginCmd);
    registry.registerTask(&webServerSetupCmd);
}

void DependencyContainer::registerLoopTasks() {
    static WiFiConfigTask wifiLoopCmd(wifiConfig);
    static ArduinoOTATask otaCmd;
    static SerialCommandsTask serialCmd(serialCmdHandler);
    
    static TriplePressCheckTask triplePressCmd(buttonHandler, resetConfigWrapper);
    
    static MqttHandlerTask mqttLoopCmd;
    static MqttReconnectTask mqttReconnectCmd(mqttHandler, configManager);
    static MqttInfoPublishTask mqttInfoCmd(mqttHandler, configManager, lastInfoPublish);
    static SerialBridge0Task serial0Cmd(serialBridge, serial0Log, serialBuffer[0], 
                                           mqttHandler, debugEnabled);
    static SerialBridge1Task serial1Cmd(serialBridge, serial1Log, serialBuffer[1], 
                                           mqttHandler, debugEnabled);
    
    registry.registerTask(&wifiLoopCmd);
    registry.registerTask(&otaCmd);
    registry.registerTask(&serialCmd);
    registry.registerTask(&triplePressCmd);
    registry.registerTask(&mqttLoopCmd);
    registry.registerTask(&mqttReconnectCmd);
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
    serialBridge.setMqttHandler(mqttHandler);
    Serial.println("Setup complete!");
}

void DependencyContainer::loop() {
    registry.loopAll();
    
    if (webServer) {
        webServer->loop();
    }
    
    if (mqttHandler) {
        for (int i = 0; i < 2; i++) {
            if (mqttHandler->shouldFlushBuffer(i)) {
                mqttHandler->flushBuffer(i);
            }
        }
    }
}

}  // namespace jrb::wifi_serial

