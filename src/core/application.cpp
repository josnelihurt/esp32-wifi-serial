#include "application.h"
#include "domain/serial/serial_bridge.h"
#include "tasks/setup/network_setup_task.h"
#include "tasks/setup/mqtt_handler_create_task.h"
#include "tasks/loop/network_loop_task.h"
#include "tasks/loop/serial_bridge_task.h"
#include "domain/network/mqtt_client.h"
#include <Arduino.h>
#include <ArduinoLog.h>
#include <WiFi.h>
namespace jrb::wifi_serial {

Application::Application() {
    // Load preferences and setup core systems
    preferencesStorage.load();
    systemInfo.logSystemInformation();
    serialBridge.setup(preferencesStorage.baudRateTty1);
    serialBridge.setLogs(serial0Log, serial1Log);

    // Create heap-allocated handlers
    serialCmdHandler = new SerialCommandHandler(
        preferencesStorage, &mqttClient, debugEnabled,
        [this]() { systemInfo.logSystemInformation(); }
    );
    buttonHandler = new ButtonHandler(
        [this]() { systemInfo.logSystemInformation(); }
    );
    otaManager = new OTAManager(preferencesStorage, otaEnabled);
    webServer = new WebConfigServer(
        preferencesStorage, serial0Log, serial1Log,
        [this](int portIndex, const String& data) {
            handleWebToSerialAndMqtt(portIndex, data);
        }
    );
}

Application::~Application() {
    delete serialCmdHandler;
    delete buttonHandler;
    delete otaManager;
    delete webServer;
}

void Application::onMqttTty0(const char* data, unsigned int length) {
    serialBridge.handleMqttToSerialAndWeb(0, data, length);
}

void Application::onMqttTty1(const char* data, unsigned int length) {
    serialBridge.handleMqttToSerialAndWeb(1, data, length);
}

void Application::handleWebToSerialAndMqtt(int portIndex, const String& data) {
    serialBridge.handleWebToSerialAndMqtt(portIndex, data);
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
    static NetworkSetupTask networkSetupCmd(wifiManager,
                                           preferences,
                                           *otaManager,
                                           systemInfo);

    auto tty0Callback = getMqttTty0Callback();
    auto tty1Callback = getMqttTty1Callback();
    static MqttHandlerCreateTask mqttCreateCmd(mqttClient,
                                               wifiClient,
                                               preferencesStorage,
                                               tty0Callback,
                                               tty1Callback);

    registry.registerTask(&networkSetupCmd);
    registry.registerTask(&mqttCreateCmd);
}

void Application::registerLoopTasks() {
    static NetworkLoopTask networkLoopCmd(wifiManager,
                                         mqttClient);

    static SerialBridge0Task serial0Cmd(serialBridge,
                                       serial0Log,
                                       serialBuffer[0],
                                       mqttClient,
                                       debugEnabled,
                                       systemInfo);
    static SerialBridge1Task serial1Cmd(serialBridge,
                                       serial1Log,
                                       serialBuffer[1],
                                       mqttClient,
                                       debugEnabled,
                                       systemInfo);

    registry.registerTask(&networkLoopCmd);
    registry.registerTask(&serial0Cmd);
    registry.registerTask(&serial1Cmd);
}

void Application::setup() {
    Log.traceln(__PRETTY_FUNCTION__);
    registerSetupTasks();
    registerLoopTasks();
    registry.setupAll();
    serialBridge.setMqttHandler(mqttClient);

    // Save preferences (inlined from SaveConfigTask)
    preferencesStorage.save();

    // Setup web config server (inlined from WebConfigServerSetupTask)
    webServer->setWiFiConfig(
        preferencesStorage.ssid, preferencesStorage.password, preferencesStorage.deviceName,
        preferencesStorage.mqttBroker, preferencesStorage.mqttPort,
        preferencesStorage.mqttUser, preferencesStorage.mqttPassword
    );
    webServer->setAPMode(wifiManager.isAPMode());
    if (wifiManager.isAPMode()) {
        webServer->setAPIP(wifiManager.getAPIP());
    }
    webServer->setup();

    // Initialize MQTT info publish timer (inlined from MqttInfoPublishTask)
    lastInfoPublish = millis();

    Log.infoln("Setup complete!");
}

void Application::loop() {
    if(buttonHandler->checkTriplePress()) {
        Log.infoln("Triple press detected! Resetting configuration and restarting...");
        preferencesStorage.clear();
        ESP.restart();
    }

    // Handle serial commands (inlined from SystemLoopTask)
    if (serialCmdHandler) {
        serialCmdHandler->handle();
    }

    // MQTT reconnection logic (inlined from MqttReconnectTask)
    if (!wifiManager.isAPMode() && preferencesStorage.mqttBroker.length() > 0 && !mqttClient.isConnected()) {
        unsigned long now = millis();
        if (now - lastMqttReconnectAttempt >= 5000) {
            lastMqttReconnectAttempt = now;
            const char* user = preferencesStorage.mqttUser.length() > 0 ? preferencesStorage.mqttUser.c_str() : nullptr;
            const char* pass = preferencesStorage.mqttPassword.length() > 0 ? preferencesStorage.mqttPassword.c_str() : nullptr;
            mqttClient.connect(preferencesStorage.mqttBroker.c_str(), preferencesStorage.mqttPort, user, pass);
        }
    }

    // MQTT info publishing logic (inlined from MqttInfoPublishTask)
    if (!wifiManager.isAPMode() && mqttClient.isConnected()) {
        static constexpr unsigned long INFO_PUBLISH_INTERVAL_MS = 30000;

        if (millis() - lastInfoPublish >= INFO_PUBLISH_INTERVAL_MS) {
            String macAddress = WiFi.macAddress();
            String ipAddress = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "Not connected";
            String ssid = WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "Not configured";

            String infoJson = "{\"device\":\"" + preferencesStorage.deviceName +
                            "\",\"ip\":\"" + ipAddress +
                            "\",\"mac\":\"" + macAddress +
                            "\",\"ssid\":\"" + ssid +
                            "\",\"mqtt\":\"" + (preferencesStorage.mqttBroker.length() > 0 ? "connected" : "disconnected") + "\"}";

            mqttClient.publishInfo(infoJson);
            lastInfoPublish = millis();
        }
    }

    registry.loopAll();

    if (webServer) {
        webServer->loop();
    } else {
        Log.error("No web config server found");
    }

    for (int i = 0; i < 2; i++) {
        if (mqttClient.shouldFlushBuffer(i)) {
            mqttClient.flushBuffer(i);
        }
    }

    delay(50);
}

}  // namespace jrb::wifi_serial

