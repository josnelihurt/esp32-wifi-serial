#pragma once

#include "config.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>

namespace jrb::wifi_serial {

class MqttClient final{
public:
    MqttClient(WiFiClient& wifiClient);
    ~MqttClient();

    void setDeviceName(const String& name);
    void setTopics(const String& tty0Rx, const String& tty0Tx, 
                   const String& tty1Rx, const String& tty1Tx);
    void setCallbacks(void (*tty0)(const char*, unsigned int),
                      void (*tty1)(const char*, unsigned int));

    bool connect(const char* broker, int port, const char* user = nullptr, 
                const char* password = nullptr);
    void disconnect();
    bool reconnect();
    void loop();

    bool publishTty0(const char* data, unsigned int length);
    bool publishTty1(const char* data, unsigned int length);
    bool publishTty0(const String& data);
    bool publishTty1(const String& data);
    bool publishInfo(const String& data);

    bool isConnected() const { return connected; }
    void setConnected(bool state) { connected = state; }
    PubSubClient* getMqttClient() const { return mqttClient; }

    void appendToBuffer(int portIndex, const char* data, unsigned int length);
    void flushBuffer(int portIndex);
    bool shouldFlushBuffer(int portIndex) const;

private:
    PubSubClient* mqttClient;
    WiFiClient* wifiClient;
    String deviceName;
    String topicTty0Rx;
    String topicTty0Tx;
    String topicTty1Rx;
    String topicTty1Tx;
    String topicInfo;
    bool connected;
    unsigned long lastReconnectAttempt;

    void (*onTty0Callback)(const char*, unsigned int);
    void (*onTty1Callback)(const char*, unsigned int);

    char mqttPublishBuffer[2][MQTT_PUBLISH_BUFFER_SIZE];
    int mqttPublishBufferLen[2];
    unsigned long lastMqttPublish[2];

    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    bool publish(int portIndex, const char* data, unsigned int length);
};

}  // namespace jrb::wifi_serial
