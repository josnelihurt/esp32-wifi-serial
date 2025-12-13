#include "application.h"
#include "domain/serial/serial_bridge.h"
#include "tasks/setup/network_setup_task.h"
#include "tasks/setup/save_config_task.h"
#include "tasks/setup/mqtt_handler_create_task.h"
#include "tasks/setup/web_config_server_setup_task.h"
#include "tasks/loop/network_loop_task.h"
#include "tasks/loop/system_loop_task.h"
#include "tasks/loop/mqtt_reconnect_task.h"
#include "tasks/loop/mqtt_info_publish_task.h"
#include "tasks/loop/serial_bridge_task.h"
#include "domain/network/mqtt_client.h"
#include <Arduino.h>
#include <ArduinoLog.h>
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


void Application::registerSetupTasks() {
    static NetworkSetupTask networkSetupCmd(container.getWiFiManager(), 
                                           container.getPreferences(),
                                           *container.getOTAManager(), 
                                           container.getSystemInfo());
    static SaveConfigTask saveConfigCmd(container.getPreferencesStorage());
    
    auto tty0Callback = getMqttTty0Callback();
    auto tty1Callback = getMqttTty1Callback();
    static MqttHandlerCreateTask mqttCreateCmd(container,
                                               container.getWiFiClient(),
                                               container.getPreferencesStorage(), 
                                               tty0Callback,
                                               tty1Callback);
    static WebConfigServerSetupTask webServerSetupCmd(*container.getWebConfigServer(), 
                                                     container.getWiFiManager(),
                                                     container.getPreferencesStorage());
    
    registry.registerTask(&networkSetupCmd);
    registry.registerTask(&saveConfigCmd);
    registry.registerTask(&mqttCreateCmd);
    registry.registerTask(&webServerSetupCmd);
}

void Application::registerLoopTasks() {
    static MqttReconnectTask mqttReconnectCmd(container.getMqttClient(), 
                                              container.getWiFiManager(),
                                              container.getPreferencesStorage());
    static NetworkLoopTask networkLoopCmd(container.getWiFiManager(), 
                                         container);
    
    static MqttInfoPublishTask mqttInfoCmd(container.getMqttClient(), 
                                          container.getWiFiManager(),
                                          container.getPreferencesStorage(), 
                                          container.getLastInfoPublish());
    static SerialBridge0Task serial0Cmd(container.getSerialBridge(), 
                                       container.getSerial0Log(), 
                                       container.getSerialBuffer(0), 
                                       container.getMqttClient(), 
                                       container.getDebugEnabled(),
                                       container.getSystemInfo());
    static SerialBridge1Task serial1Cmd(container.getSerialBridge(), 
                                       container.getSerial1Log(), 
                                       container.getSerialBuffer(1), 
                                       container.getMqttClient(), 
                                       container.getDebugEnabled(),
                                       container.getSystemInfo());
    static SystemLoopTask systemLoopCmd(container.getSerialCommandHandler());
    
    registry.registerTask(&mqttReconnectCmd);
    registry.registerTask(&networkLoopCmd);
    registry.registerTask(&mqttInfoCmd);
    registry.registerTask(&serial0Cmd);
    registry.registerTask(&serial1Cmd);
    registry.registerTask(&systemLoopCmd);
}

void Application::setup() {
    Log.traceln(__PRETTY_FUNCTION__);
    registerSetupTasks();
    registerLoopTasks();
    registry.setupAll();
    container.getSerialBridge().setMqttHandler(container.getMqttClient());
    Log.infoln("Setup complete!");
}

void Application::loop() {
    if(container.getButtonHandler()->checkTriplePress()) {
        Log.infoln("Triple press detected! Resetting configuration and restarting...");
        container.getPreferencesStorage().clear();
        ESP.restart();
    }
    registry.loopAll();
    
    if (container.getWebConfigServer()) {
        container.getWebConfigServer()->loop();
    } else {
        Log.error("No web config server found");
    }
    
    for (int i = 0; i < 2; i++) {
        if (container.getMqttClient().shouldFlushBuffer(i)) {
            container.getMqttClient().flushBuffer(i);
        }
    }
    
    delay(50);
}

}  // namespace jrb::wifi_serial

