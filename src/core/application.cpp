#include "application.h"
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
#include "domain/network/mqtt_client.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

Application::Application(DependencyContainer& container)
    : container(container) {
    container.createHandlers([this](int portIndex, const String& data) {
        handleWebToSerialAndMqtt(portIndex, data);
    });
}

void Application::onMqttTty0(const char* data, unsigned int length) {
    container.getSerialBridge().handleMqttToSerialAndWeb(0, data, length);
}

void Application::onMqttTty1(const char* data, unsigned int length) {
    container.getSerialBridge().handleMqttToSerialAndWeb(1, data, length);
}

void Application::onResetConfig() {
    container.getPreferencesStorage().clear();
    container.getWiFiManager().resetSettings();
}

void Application::handleWebToSerialAndMqtt(int portIndex, const String& data) {
    container.getSerialBridge().handleWebToSerialAndMqtt(portIndex, data);
}

std::function<void(const char*, unsigned int)> Application::getMqttTty0Callback() {
    return [this](const char* data, unsigned int length) {
        onMqttTty0(data, length);
    };
}

std::function<void(const char*, unsigned int)> Application::getMqttTty1Callback() {
    return [this](const char* data, unsigned int length) {
        onMqttTty1(data, length);
    };
}

std::function<void()> Application::getResetConfigCallback() {
    return [this]() {
        onResetConfig();
    };
}

void Application::registerSetupTasks() {
    static HardwareSetupTask hardwareSetupCmd(container.getSerialBridge());
    static ConfigSetupTask configSetupCmd(container.getPreferencesStorage(), 
                                         *container.getSystemInfo());
    
    auto resetCallback = getResetConfigCallback();
    static TriplePressCheckSetupTask triplePressCheckCmd(*container.getButtonHandler(), 
                                                         resetCallback);
    
    static NetworkSetupTask networkSetupCmd(container.getWiFiManager(), 
                                           container.getPreferences(),
                                           *container.getOTAManager(), 
                                           *container.getSystemInfo());
    static WiFiConfigAPModeWaitTask wifiAPWaitCmd(container.getWiFiManager());
    static ConfigManagerSyncTask configSyncCmd(container.getWiFiManager(), 
                                              container.getPreferencesStorage());
    static SaveConfigTask saveConfigCmd(container.getPreferencesStorage());
    
    auto tty0Callback = getMqttTty0Callback();
    auto tty1Callback = getMqttTty1Callback();
    static MqttHandlerCreateTask mqttCreateCmd(container,
                                               container.getWiFiClient(),
                                               container.getPreferencesStorage(), 
                                               tty0Callback,
                                               tty1Callback);
    static WebConfigServerSetupTask webServerSetupCmd(*container.getWebConfigServer(), 
                                                     container.getWiFiManager());
    
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

void Application::registerLoopTasks() {
    // NOTE: MqttReconnectTask receives IMqttClient* which may be nullptr at registration time.
    // This is OK because it checks for nullptr before use. However, NetworkLoopTask MUST
    // receive DependencyContainer& to get MqttClient dynamically, as MqttClient is created
    // AFTER loop tasks are registered (in MqttHandlerCreateTask::setup()).
    static MqttReconnectTask mqttReconnectCmd(container.getMqttClient(), 
                                              container.getPreferencesStorage());
    static NetworkLoopTask networkLoopCmd(container.getWiFiManager(), 
                                         container);
    static SystemLoopTask systemLoopCmd(container.getSerialCommandHandler());
    
    auto resetCallback = getResetConfigCallback();
    static TriplePressCheckTask triplePressCmd(container.getButtonHandler(), 
                                              resetCallback);
    
    static MqttInfoPublishTask mqttInfoCmd(container.getMqttClient(), 
                                          container.getPreferencesStorage(), 
                                          container.getLastInfoPublish());
    static SerialBridge0Task serial0Cmd(container.getSerialBridge(), 
                                       container.getSerial0Log(), 
                                       container.getSerialBuffer(0), 
                                       container.getMqttClient(), 
                                       container.getDebugEnabled());
    static SerialBridge1Task serial1Cmd(container.getSerialBridge(), 
                                       container.getSerial1Log(), 
                                       container.getSerialBuffer(1), 
                                       container.getMqttClient(), 
                                       container.getDebugEnabled());
    
    registry.registerTask(&mqttReconnectCmd);
    registry.registerTask(&networkLoopCmd);
    registry.registerTask(&systemLoopCmd);
    registry.registerTask(&triplePressCmd);
    registry.registerTask(&mqttInfoCmd);
    registry.registerTask(&serial0Cmd);
    registry.registerTask(&serial1Cmd);
}

void Application::initialize() {
    registerSetupTasks();
    registerLoopTasks();
}

void Application::setup() {
    registry.setupAll();
    container.getSerialBridge().setMqttHandler(container.getMqttClient());
    Serial.println("Setup complete!");
}

void Application::loop() {
    registry.loopAll();
    
    if (container.getWebConfigServer()) {
        container.getWebConfigServer()->loop();
    }
    
    if (container.getMqttClient()) {
        for (int i = 0; i < 2; i++) {
            if (container.getMqttClient()->shouldFlushBuffer(i)) {
                container.getMqttClient()->flushBuffer(i);
            }
        }
    }
}

}  // namespace jrb::wifi_serial

