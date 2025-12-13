#include "mqtt_client.h"
#include "config.h"
#include <cstring>
#include <ArduinoLog.h>

namespace jrb::wifi_serial {

// Static pointer to MqttClient instance for C-style callback from PubSubClient.
// PubSubClient requires a C function pointer, so we use this to access instance data.
static MqttClient* g_mqttInstance{};

MqttClient::MqttClient(WiFiClient& wifiClient)
    : mqttClient{new PubSubClient(wifiClient)}
    , wifiClient{&wifiClient}
    , deviceName{DEFAULT_DEVICE_NAME}
    , connected{false}
    , lastReconnectAttempt{0}
    , onTty0Callback{nullptr}
    , onTty1Callback{nullptr}
    , mqttPublishBufferLen{0, 0}
    , lastMqttPublish{0, 0}
{
    mqttClient->setBufferSize(MQTT_BUFFER_SIZE);
    mqttClient->setCallback(mqttCallback);
    mqttClient->setKeepAlive(60);
    mqttClient->setSocketTimeout(15);
    g_mqttInstance = this;
}

MqttClient::~MqttClient() {
    if (g_mqttInstance == this) {
        g_mqttInstance = nullptr;
    }
    if (!mqttClient) return;
    delete mqttClient;
}

void MqttClient::setDeviceName(const String& name) {
    deviceName = name;
}

void MqttClient::setTopics(const String& tty0Rx, const String& tty0Tx,
                           const String& tty1Rx, const String& tty1Tx) {
    topicTty0Rx = tty0Rx;
    topicTty0Tx = tty0Tx;
    topicTty1Rx = tty1Rx;
    topicTty1Tx = tty1Tx;
    // Generate info topic from tty0Rx topic (replace /ttyS0/rx with /info)
    topicInfo = tty0Rx;
    topicInfo.replace("/ttyS0/rx", "/info");
    if (topicInfo.endsWith("/rx")) {
        int lastSlash = topicInfo.lastIndexOf('/');
        topicInfo = topicInfo.substring(0, lastSlash) + "/info";
    }
    Log.infoln("MQTT info topic set to: %s", topicInfo.c_str());
}

void MqttClient::setCallbacks(void (*tty0)(const char*, unsigned int),
                               void (*tty1)(const char*, unsigned int)) {
    onTty0Callback = tty0;
    onTty1Callback = tty1;
}

bool MqttClient::connect(const char* broker, int port, const char* user,
                         const char* password) {
    mqttClient->setServer(broker, port);

    String clientId = "ESP32-C3-" + deviceName + "-" + String(random(0xffff), HEX);

    Log.infoln("MQTT connecting to %s:%d (ClientID: %s)", broker, port, clientId.c_str());

    bool result = false;
    if (user && password) {
        result = mqttClient->connect(clientId.c_str(), user, password);
    } else {
        result = mqttClient->connect(clientId.c_str());
    }

    connected = result;
    if (!result) {
        Log.errorln("MQTT connection failed! State: %d", mqttClient->state());
        return false;
    }

    Log.infoln("MQTT connected successfully!");
    
    if (topicTty0Tx.length() > 0) {
        mqttClient->subscribe(topicTty0Tx.c_str(), 1);
        mqttClient->loop();
        delay(10);
        mqttClient->loop();
        if (topicTty0Rx.length() > 0) {
            String initMsg0 = "Init: " + deviceName + " ttyS0 connected";
            publishTty0(initMsg0);
        }
    }
    
    if (topicTty1Tx.length() > 0) {
        mqttClient->subscribe(topicTty1Tx.c_str(), 1);
        mqttClient->loop();
        delay(10);
        mqttClient->loop();
        if (topicTty1Rx.length() > 0) {
            String initMsg1 = "Init: " + deviceName + " ttyS1 connected";
            publishTty1(initMsg1);
        }
    }
    
    return true;
}

void MqttClient::disconnect() {
    if (!mqttClient) {
        connected = false;
        return;
    }

    Log.infoln("MQTT disconnecting...");
    mqttClient->disconnect();
    connected = false;
    Log.infoln("MQTT disconnected");
}

bool MqttClient::reconnect() {
    if (!mqttClient) return false;
    connected = mqttClient->connected();
    return connected;
}

void MqttClient::loop() {
    if (!mqttClient) return;
    
    bool wasConnected = connected;
    
    // Call loop() multiple times to ensure all messages are processed
    mqttClient->loop();
    mqttClient->loop();
    mqttClient->loop();
    
    connected = mqttClient->connected();

    if (wasConnected && !connected) {
        Log.warningln("MQTT connection lost!");
        connected = false;
    }

    if (!wasConnected && connected) {
        Log.infoln("MQTT reconnected successfully!");
        if (topicTty0Tx.length() > 0) {
            mqttClient->subscribe(topicTty0Tx.c_str(), 1);
            mqttClient->loop();
            delay(10);
            mqttClient->loop();
        }
        if (topicTty1Tx.length() > 0) {
            mqttClient->subscribe(topicTty1Tx.c_str(), 1);
            mqttClient->loop();
            delay(10);
            mqttClient->loop();
        }
    }
}

bool MqttClient::publish(int portIndex, const char* data, unsigned int length) {
    if (!mqttClient || portIndex < 0 || portIndex > 1) return false;
    
    const String& topic = (portIndex == 0) ? topicTty0Rx : topicTty1Rx;
    if (topic.length() == 0) return false;
    
    connected = mqttClient->connected();
    if (!connected) return false;
    
    bool result = mqttClient->publish(topic.c_str(), (uint8_t*)data, length, false);
    if (!result) {
        connected = mqttClient->connected();
        if (!connected) {
            Log.errorln("MQTT publish failed, connection lost. State: %d", mqttClient->state());
        }
    }
    return result;
}

bool MqttClient::publishTty0(const char* data, unsigned int length) {
    return publish(0, data, length);
}

bool MqttClient::publishTty1(const char* data, unsigned int length) {
    return publish(1, data, length);
}

bool MqttClient::publishTty0(const String& data) {
    return publishTty0(data.c_str(), data.length());
}

bool MqttClient::publishTty1(const String& data) {
    return publishTty1(data.c_str(), data.length());
}

bool MqttClient::publishInfo(const String& data) {
    if (!mqttClient) {
        Log.errorln("MQTT publishInfo failed: mqttClient is null");
        return false;
    }

    if (topicInfo.length() == 0) {
        Log.errorln("MQTT publishInfo failed: topicInfo is empty");
        return false;
    }

    connected = mqttClient->connected();
    if (!connected) {
        Log.warningln("MQTT publishInfo failed: not connected");
        return false;
    }

    Log.traceln("MQTT publishing info to %s (%d bytes)", topicInfo.c_str(), data.length());

    bool result = mqttClient->publish(topicInfo.c_str(), (uint8_t*)data.c_str(), data.length(), false);
    if (!result) {
        Log.errorln("MQTT publishInfo failed! State: %d", mqttClient->state());
        connected = mqttClient->connected();
    } else {
        Log.traceln("MQTT info published successfully");
    }
    return result;
}

void MqttClient::appendToBuffer(int portIndex, const char* data, unsigned int length) {
    if (portIndex < 0 || portIndex > 1 || length == 0) return;
    
    for (unsigned int i = 0; i < length; i++) {
        if (mqttPublishBufferLen[portIndex] >= MQTT_PUBLISH_BUFFER_SIZE - 1) {
            flushBuffer(portIndex);
        }
        if (mqttPublishBufferLen[portIndex] < MQTT_PUBLISH_BUFFER_SIZE - 1) {
            mqttPublishBuffer[portIndex][mqttPublishBufferLen[portIndex]++] = data[i];
        }
    }
    
    if (mqttPublishBufferLen[portIndex] >= MQTT_PUBLISH_MIN_CHARS) {
        flushBuffer(portIndex);
    }
}

void MqttClient::flushBuffer(int portIndex) {
    if (portIndex < 0 || portIndex > 1) return;
    if (mqttPublishBufferLen[portIndex] == 0 || !connected) return;
    
    if (portIndex == 0) {
        publishTty0(mqttPublishBuffer[portIndex], mqttPublishBufferLen[portIndex]);
    } else {
        publishTty1(mqttPublishBuffer[portIndex], mqttPublishBufferLen[portIndex]);
    }
    
    mqttPublishBufferLen[portIndex] = 0;
    lastMqttPublish[portIndex] = millis();
}

bool MqttClient::shouldFlushBuffer(int portIndex) const {
    if (portIndex < 0 || portIndex > 1) return false;
    if (mqttPublishBufferLen[portIndex] == 0) return false;
    
    unsigned long now = millis();
    return (now - lastMqttPublish[portIndex] >= MQTT_PUBLISH_INTERVAL_MS);
}

void MqttClient::mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (!g_mqttInstance) return;
    if (length >= 512) return;

    static char buffer[512];
    memcpy(buffer, payload, length);
    buffer[length] = '\0';

    String topicStr = String(topic);
    
    if ((topicStr == g_mqttInstance->topicTty0Tx || strcmp(topic, g_mqttInstance->topicTty0Tx.c_str()) == 0)) {
        if (g_mqttInstance->onTty0Callback) {
            g_mqttInstance->onTty0Callback(buffer, length);
        }
        return;
    }
    
    if ((topicStr == g_mqttInstance->topicTty1Tx || strcmp(topic, g_mqttInstance->topicTty1Tx.c_str()) == 0)) {
        if (g_mqttInstance->onTty1Callback) {
            g_mqttInstance->onTty1Callback(buffer, length);
        }
        return;
    }
}

}  // namespace jrb::wifi_serial

