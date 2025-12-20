#pragma once

#include "config.h"
#include "domain/config/preferences_storage_policy.h"
#include "domain/messaging/buffered_stream.hpp"
#include "domain/messaging/mqtt_flush_policy.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <functional>
#include <memory>
#include <nonstd/span.hpp>
#include <string>
#include <vector>

namespace jrb::wifi_serial {

class MqttClient final {
public:
  MqttClient(WiFiClient &wifiClient,
             PreferencesStorageDefault &preferencesStorage);
  ~MqttClient();

  // Registers callbacks for Rx topics.
  void setCallbacks(void (*tty0)(const nonstd::span<const uint8_t> &),
                    void (*tty1)(const nonstd::span<const uint8_t> &));

  bool connect(const char *broker, int port, const char *user = nullptr,
               const char *password = nullptr);
  void disconnect();
  bool reconnect();
  void loop();

  bool publishInfo(const std::string &data);

  // Connection state query.
  bool isConnected() const { return connected; }
  void setConnected(bool state) { connected = state; }

  void appendToTty0Buffer(const nonstd::span<const uint8_t> &data);
  void appendToTty1Buffer(const nonstd::span<const uint8_t> &data);
  BufferedStream<MqttFlushPolicy> &getTty0Stream() { return tty0Stream; }
  BufferedStream<MqttFlushPolicy> &getTty1Stream() { return tty1Stream; }

private:
  std::unique_ptr<PubSubClient> mqttClient;
  PreferencesStorageDefault &preferencesStorage;
  WiFiClient *wifiClient;
  std::string topicTty0Rx, topicTty0Tx;
  std::string topicTty1Rx, topicTty1Tx;
  std::string topicInfo;
  bool connected;
  unsigned long lastReconnectAttempt;

  void (*onTty0Callback)(const nonstd::span<const uint8_t> &);
  void (*onTty1Callback)(const nonstd::span<const uint8_t> &);

  // Pending buffers for cross-task data transfer (web task → main loop)
  std::vector<uint8_t> tty0PendingBuffer;
  std::vector<uint8_t> tty1PendingBuffer;

  BufferedStream<MqttFlushPolicy> tty0Stream;
  unsigned long tty0LastFlushMillis;
  BufferedStream<MqttFlushPolicy> tty1Stream;
  unsigned long tty1LastFlushMillis;

  void subscribeToConfiguredTopics();
  void handleConnectionStateChange(bool wasConnected);
  void flushBuffersIfNeeded();
  void flushBuffer(std::vector<char> &buffer, const std::string &topic,
                   unsigned long &lastFlushMillis, const char *bufferName);
  void setTopics(const std::string &tty0Rx, const std::string &tty0Tx,
                 const std::string &tty1Rx, const std::string &tty1Tx);
  void mqttCallback(char *topic, byte *payload, unsigned int length);
};

} // namespace jrb::wifi_serial