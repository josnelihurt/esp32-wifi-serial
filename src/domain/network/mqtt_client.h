#pragma once

#include "config.h"
#include "interfaces/imqtt_client.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>

namespace jrb::wifi_serial {

class MqttClient final : public IMqttClient {
public:
    MqttClient(WiFiClient& wifiClient);
    ~MqttClient();

    void setDeviceName(const String& name) override;
    void setTopics(const String& tty0Rx, const String& tty0Tx, 
                   const String& tty1Rx, const String& tty1Tx) override;
    void setCallbacks(void (*tty0)(const char*, unsigned int),
                      void (*tty1)(const char*, unsigned int)) override;

    bool connect(const char* broker, int port, const char* user = nullptr, 
                const char* password = nullptr) override;
    void disconnect() override;
    bool reconnect() override;
    void loop() override;

    bool publishTty0(const char* data, unsigned int length) override;
    bool publishTty1(const char* data, unsigned int length) override;
    bool publishTty0(const String& data) override;
    bool publishTty1(const String& data) override;
    bool publishInfo(const String& data) override;

    bool isConnected() const override { return connected; }
    void setConnected(bool state) override { connected = state; }
    PubSubClient* getMqttClient() const { return mqttClient; }

    void appendToBuffer(int portIndex, const char* data, unsigned int length) override;
    void flushBuffer(int portIndex) override;
    bool shouldFlushBuffer(int portIndex) const override;

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
