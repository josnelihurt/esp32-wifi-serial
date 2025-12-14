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
    tty0Buffer.reserve(SERIAL_BUFFER_SIZE);
    tty1Buffer.reserve(SERIAL_BUFFER_SIZE);
    tty0LastFlushMillis = millis();
    tty1LastFlushMillis = millis();
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
        [this](const String& data) {
            //Handle web to serial and mqtt
            if(debugEnabled) {
                Log.infoln("$web->ttyS0$%s", data.c_str());
            }
            std::vector<char> dataVector(data.begin(), data.end());
            mqttClient.appendToTty0Buffer(dataVector);
        },
        [this](const String& data) {
            //Handle web to serial and mqtt
            if(debugEnabled) {
                Log.infoln("$web->ttyS1$%s", data.c_str());
            }
            std::vector<char> dataVector(data.begin(), data.end());
            mqttClient.appendToTty1Buffer(dataVector);
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
    serialBridge.writeSerial0(data, length);
}

void Application::onMqttTty1(const char* data, unsigned int length) {
    serialBridge.writeSerial1(data, length);
    if(debugEnabled) {
        Log.infoln("$mqtt->ttyS1$%s", data);
    }
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


bool Application::handleSpecialCharacter(char c) {
    static bool isSpecialCharacter = false;
    if (c == CMD_PREFIX) {
        isSpecialCharacter = true;
        Log.infoln("Ctrl+Y");
        return true;
    }
    if (!isSpecialCharacter) return false;
    isSpecialCharacter = false;
    switch (c) {
        case CMD_INFO:
            systemInfo.logSystemInformation();
            break;
        case CMD_DEBUG:
            Log.infoln("Debug %s", debugEnabled ? "enabled" : "disabled");
            debugEnabled = !debugEnabled;
            break;
        default:
            Log.errorln("Unknown special character: %c", c);
            break;
    }
}

void Application::handleSerialPort0() {
    while (Serial.available() > 0) {
        tty0Buffer.push_back(Serial.read());
        if(debugEnabled && !handleSpecialCharacter(tty0Buffer.back())){
            Serial.print(tty0Buffer.back());
        }
    }
    if(tty0Buffer.empty()) return; // No data to flush
    if(millis() - tty0LastFlushMillis < 50) return; // Not enough time has passed since last flush
    tty0LastFlushMillis = millis();
    if(mqttClient.isConnected()) {
        mqttClient.appendToTty0Buffer(tty0Buffer);
    }
    tty0Buffer.clear();
}

void Application::handleSerialPort1() {
    while (Serial.available() > 0) {
        tty1Buffer.push_back(Serial.read());
        if(debugEnabled){
            Serial.print(tty1Buffer.back());
        }
    }
    if(tty1Buffer.empty()) return; // No data to flush
    if(millis() - tty0LastFlushMillis < 50) return; // Not enough time has passed since last flush
    tty0LastFlushMillis = millis();
    if(mqttClient.isConnected()) {
        mqttClient.appendToTty1Buffer(tty1Buffer);
    }
    tty1Buffer.clear();
}

}  // namespace jrb::wifi_serial

