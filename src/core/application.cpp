#include "application.h"
#include "domain/serial/serial_bridge.h"
#include "domain/network/mqtt_client.h"
#include <Arduino.h>
#include <ArduinoLog.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

namespace jrb::wifi_serial {

// Static instance pointer for MQTT callbacks
Application* Application::s_instance = nullptr;

Application::Application() {
    // Set static instance for MQTT callbacks
    s_instance = this;

    // Load preferences and setup core systems
    preferencesStorage.load();
    systemInfo.logSystemInformation();
    serialBridge.setup(preferencesStorage.baudRateTty1);
    serialBridge.setLogs(serial0Log, serial1Log);

    // Create heap-allocated handlers
    serialCmdHandler = new SerialCommandHandler(
        preferencesStorage, mqttClient, debugEnabled,
        [this]() { systemInfo.logSystemInformation(); }
    );
    otaManager = new OTAManager(preferencesStorage, otaEnabled);
    webServer = new WebConfigServer(
        preferencesStorage, serial0Log, serial1Log,
        [this](int portIndex, const String& data) {
            handleWebToSerialAndMqtt(portIndex, data);
        }
    );

    // Initialize SSH server (runs in its own FreeRTOS task)
    sshServer = new SSHServer(preferencesStorage, systemInfo);
    sshServer->setSerialCallbacks(
        [this](const char* data, int length) {
            serialBridge.writeSerial1(data, length);
        },
        [this](char* buffer, int maxLen) {
            return serialBridge.readSerial1(buffer, maxLen);
        }
    );
}

Application::~Application() {
    delete serialCmdHandler;
    delete otaManager;
    delete webServer;
    delete sshServer;
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

// Static MQTT callback wrappers for C-style function pointers
void Application::mqttTty0Wrapper(const char* data, unsigned int length) {
    if (s_instance == nullptr) return;
    s_instance->onMqttTty0(data, length);
}

void Application::mqttTty1Wrapper(const char* data, unsigned int length) {
    if (s_instance == nullptr) return;
    s_instance->onMqttTty1(data, length);
}

void Application::setupMqttCallbacks() {
    mqttClient.setDeviceName(preferencesStorage.deviceName);
    mqttClient.setTopics(preferencesStorage.topicTty0Rx, preferencesStorage.topicTty0Tx,
                         preferencesStorage.topicTty1Rx, preferencesStorage.topicTty1Tx);
    mqttClient.setCallbacks(mqttTty0Wrapper, mqttTty1Wrapper);

    if (preferencesStorage.mqttBroker.length() > 0) {
        const char* user = preferencesStorage.mqttUser.length() > 0 ? preferencesStorage.mqttUser.c_str() : nullptr;
        const char* pass = preferencesStorage.mqttPassword.length() > 0 ? preferencesStorage.mqttPassword.c_str() : nullptr;
        mqttClient.connect(preferencesStorage.mqttBroker.c_str(), preferencesStorage.mqttPort, user, pass);
    }
}

void Application::setup() {
    Log.traceln(__PRETTY_FUNCTION__);

    // Network setup (from NetworkSetupTask)
    wifiManager.setup();
    otaManager->setup();
    systemInfo.logSystemInformation();

    // MQTT setup (from MqttHandlerCreateTask)
    setupMqttCallbacks();
    // Save preferences
    preferencesStorage.save();

    // Web server setup
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

    // SSH server setup (after network is ready)
    if (sshServer) {
        sshServer->setup();
    }

    // Initialize timers
    lastInfoPublish = millis();

    Log.infoln("Setup complete!");
}

void Application::loop() {
    if(buttonHandler.checkTriplePress()) {
        Log.infoln("Triple press detected! Resetting configuration and restarting...");
        preferencesStorage.clear();
        ESP.restart();
    }

    if (serialCmdHandler) {
        serialCmdHandler->handle();
    }

    wifiManager.loop();
    mqttClient.loop();
    ArduinoOTA.handle();

    reconnectMqttIfNeeded();
    publishInfoIfNeeded();

    handleSerialPort0();
    handleSerialPort1();

    // SSH server runs in its own FreeRTOS task, so no loop() call needed

    for (int i = 0; i < 2; i++) {
        if (mqttClient.shouldFlushBuffer(i)) {
            mqttClient.flushBuffer(i);
        }
    }

    delay(50);
}

void Application::reconnectMqttIfNeeded() {
    if (!wifiManager.isAPMode() && preferencesStorage.mqttBroker.length() > 0 && !mqttClient.isConnected()) {
        unsigned long now = millis();
        if (now - lastMqttReconnectAttempt >= 5000) {
            lastMqttReconnectAttempt = now;
            const char* user = preferencesStorage.mqttUser.length() > 0 ? preferencesStorage.mqttUser.c_str() : nullptr;
            const char* pass = preferencesStorage.mqttPassword.length() > 0 ? preferencesStorage.mqttPassword.c_str() : nullptr;
            mqttClient.connect(preferencesStorage.mqttBroker.c_str(), preferencesStorage.mqttPort, user, pass);
        }
    }
}

void Application::publishInfoIfNeeded() {
    if (!wifiManager.isAPMode() && mqttClient.isConnected()) {
        static constexpr unsigned long INFO_PUBLISH_INTERVAL_MS = 30000;

        if (millis() - lastInfoPublish >= INFO_PUBLISH_INTERVAL_MS) {
            String macAddress = WiFi.macAddress();
            String ipAddress = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "Not connected";
            String ssid = WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "Not configured";
            mqttClient.publishInfo(preferencesStorage.serialize(ipAddress, macAddress, ssid));
            lastInfoPublish = millis();
        }
    }
}

void Application::handleSerialPort0() {
    if (!serialBridge.available0()) return;

    int len = serialBridge.readSerial0(serialBuffer[0], SERIAL_BUFFER_SIZE);
    if (len <= 0) return;

    if (debugEnabled) {
        Log.traceln("[DEBUG TTY0] %.*s", len, serialBuffer[0]);
    }

    // Command detection for ttyS0 - use a clean output buffer to avoid corruption
    static bool cmdPrefixReceived = false;
    static char cleanBuffer[SERIAL_BUFFER_SIZE];
    int cleanLen = 0;

    for (int i = 0; i < len; i++) {
        char c = serialBuffer[0][i];

        if (cmdPrefixReceived) {
            cmdPrefixReceived = false;

            if (c >= 32 && c <= 126) {
                Log.traceln("DEBUG: After Ctrl+Y, got char: 0x%X ('%c')", (int)c, c);
            } else {
                Log.traceln("DEBUG: After Ctrl+Y, got char: 0x%X", (int)c);
            }

            if (c == CMD_INFO || c == 'I') {
                systemInfo.logSystemInformation();
                // Skip both command prefix and command char
                continue;
            } else if (c == CMD_DEBUG || c == 'D') {
                Log.infoln("Command detected: Ctrl+Y + d (Toggle Debug)");
                debugEnabled = !debugEnabled;
                Log.infoln("Debug %s", debugEnabled ? "enabled" : "disabled");
                // Skip both command prefix and command char
                continue;
            }
            // If not a recognized command, pass through the character
            cleanBuffer[cleanLen++] = c;
            continue;
        }

        if (c == CMD_PREFIX) {
            Log.traceln("DEBUG: Ctrl+Y (command prefix) detected");
            cmdPrefixReceived = true;
            // Don't add command prefix to output buffer
            continue;
        }

        // Normal character - add to clean buffer
        cleanBuffer[cleanLen++] = c;
    }

    // Send clean buffer without command sequences
    if (cleanLen > 0) {
        serialBridge.handleSerialToMqttAndWeb(0, cleanBuffer, cleanLen);
    }
}

void Application::handleSerialPort1() {
    if (!serialBridge.available1()) return;

    int len = serialBridge.readSerial1(serialBuffer[1], SERIAL_BUFFER_SIZE);
    if (len <= 0) return;

    if (debugEnabled) {
        Log.traceln("[DEBUG TTY1] %.*s", len, serialBuffer[1]);
    }

    serialBridge.handleSerialToMqttAndWeb(1, serialBuffer[1], len);
}

}  // namespace jrb::wifi_serial

