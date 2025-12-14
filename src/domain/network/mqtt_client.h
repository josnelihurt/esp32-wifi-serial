#pragma once

#include "config.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <functional>
#include <vector>

namespace jrb::wifi_serial {

class MqttClient final {
public:
  MqttClient(WiFiClient &wifiClient);
  ~MqttClient();

  // Sets device name used in MQTT topics and info messages.
  void setDeviceName(const String &name);

  // Configures the four serial topic pairs (Rx/Tx for tty0/tty1).
  void setTopics(const String &tty0Rx, const String &tty0Tx,
                 const String &tty1Rx, const String &tty1Tx);

  // Registers callbacks for Rx topics.
  void setCallbacks(void (*tty0)(const char *, unsigned int),
                    void (*tty1)(const char *, unsigned int));

  bool connect(const char *broker, int port, const char *user = nullptr,
               const char *password = nullptr);
  void disconnect();
  bool reconnect();
  void loop();

  bool publishInfo(const String &data);

  // Connection state query.
  bool isConnected() const { return connected; }
  void setConnected(bool state) { connected = state; }

  PubSubClient *getMqttClient() const { return mqttClient; }

  void appendToTty0Buffer(const std::vector<char> &data);
  void appendToTty1Buffer(const std::vector<char> &data);

private:
  PubSubClient *mqttClient;
  WiFiClient *wifiClient;
  String deviceName;
  String topicTty0Rx, topicTty0Tx;
  String topicTty1Rx, topicTty1Tx;
  String topicInfo;
  bool connected;
  unsigned long lastReconnectAttempt;

  void (*onTty0Callback)(const char *, unsigned int);
  void (*onTty1Callback)(const char *, unsigned int);

  std::vector<char> tty0Buffer;
  std::vector<char> tty1Buffer;
  unsigned long tty0LastFlushMillis{0};
  unsigned long tty1LastFlushMillis{0};

  void appendToBufferWithFlush(std::vector<char> &buffer,
                               const std::vector<char> &data,
                               std::function<void()> flushFunction,
                               const char* func = __PRETTY_FUNCTION__);
  void flushBuffer(std::vector<char> &buffer, const String &topic, unsigned long &lastFlushMillis, const char* func = __PRETTY_FUNCTION__);
  void flushTty0Buffer();
  void flushTty1Buffer();
  static void mqttCallback(char *topic, byte *payload, unsigned int length);
};

} // namespace jrb::wifi_serial