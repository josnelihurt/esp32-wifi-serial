#include "mqtt_client.h"
#include "config.h"
#include "domain/config/preferences_storage.h"
#include "infrastructure/logging/logger.h"
#include "infrastructure/types.hpp"
#include "infrastructure/mqttt/pub_sub_client_policy.h"
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string_view>

#ifndef ESP_PLATFORM
#include "infrastructure/platform/arduino_compat.h"
#endif

namespace jrb::wifi_serial {

namespace internal {
// Constants
namespace {
constexpr size_t MQTT_CALLBACK_BUFFER_SIZE = 512;
constexpr uint8_t MQTT_QOS_LEVEL = 1;
constexpr uint16_t MQTT_KEEPALIVE_SEC = 60;
constexpr uint16_t MQTT_SOCKET_TIMEOUT_SEC = 15;
constexpr uint8_t MQTT_SUBSCRIPTION_DELAY_MS = 10;
constexpr uint8_t MQTT_LOOP_ITERATIONS = 3;
} // namespace

template <typename PubSubClientPolicy>
MqttClient<PubSubClientPolicy>::MqttClient(
    PubSubClientPolicy &mqttClient,
    wifi_serial::PreferencesStorage &preferencesStorage)
    : mqttClient{mqttClient}, preferencesStorage{preferencesStorage},
      connected{false}, lastReconnectAttempt{0}, onTty0Callback{nullptr},
      onTty1Callback{nullptr},
      tty0Stream{MqttFlushPolicy<PubSubClientPolicy>{mqttClient, topicTty0Tx}, "tty0"},
      tty1Stream{MqttFlushPolicy<PubSubClientPolicy>{mqttClient, topicTty1Tx}, "tty1"},
      tty0LastFlushMillis{0}, tty1LastFlushMillis{0} {

  mqttClient.setBufferSize(MQTT_BUFFER_SIZE);
  mqttClient.setCallback([&](char *topic, byte *payload, unsigned int length) {
    mqttCallback(topic, payload, length);
  });
  mqttClient.setKeepAlive(MQTT_KEEPALIVE_SEC);
  mqttClient.setSocketTimeout(MQTT_SOCKET_TIMEOUT_SEC);
  setTopics(preferencesStorage.topicTty0Rx, preferencesStorage.topicTty0Tx,
            preferencesStorage.topicTty1Rx, preferencesStorage.topicTty1Tx);
}

template <typename PubSubClientPolicy>
void MqttClient<PubSubClientPolicy>::setTopics(const types::string &tty0Rx,
                                               const types::string &tty0Tx,
                                               const types::string &tty1Rx,
                                               const types::string &tty1Tx) {
  topicTty0Rx = tty0Rx;
  topicTty0Tx = tty0Tx;
  topicTty1Rx = tty1Rx;
  topicTty1Tx = tty1Tx;
  // Generate info topic from tty0Rx topic (replace /ttyS0/rx with /info)
  topicInfo = tty0Rx;
  size_t pos = topicInfo.find("/ttyS0/rx");
  if (pos != types::string::npos) {
    topicInfo.replace(pos, 9, "/info");
  } else if (topicInfo.size() >= 3 &&
             topicInfo.substr(topicInfo.size() - 3) == "/rx") {
    size_t lastSlash = topicInfo.rfind('/');
    if (lastSlash != types::string::npos) {
      topicInfo = topicInfo.substr(0, lastSlash) + "/info";
    }
  }
  LOG_INFO("MQTT info topic set to: %s", topicInfo.c_str());
}

template <typename PubSubClientPolicy>
void MqttClient<PubSubClientPolicy>::setCallbacks(
    void (*tty0)(const types::span<const uint8_t> &),
    void (*tty1)(const types::span<const uint8_t> &)) {
  onTty0Callback = tty0;
  onTty1Callback = tty1;
}

template <typename PubSubClientPolicy>
bool MqttClient<PubSubClientPolicy>::connect(const char *broker, int port,
                                             const char *user,
                                             const char *password) {
  mqttClient.setServer(broker, port);

  std::ostringstream oss;
  oss << "ESP32-C3-" << preferencesStorage.deviceName << "-" << std::hex
      << random(0xffff);
  types::string clientId = oss.str();

  LOG_INFO("MQTT connecting to %s:%d (ClientID: %s)", broker, port,
           clientId.c_str());

  bool result = false;
  if (user && password) {
    result = mqttClient.connect(clientId.c_str(), user, password);
  } else {
    result = mqttClient.connect(clientId.c_str());
  }

  connected = result;
  if (!result) {
    LOG_ERROR("MQTT connection failed! State: %d", mqttClient.state());
    return false;
  }

  LOG_INFO("MQTT connected successfully!");

  subscribeToConfiguredTopics();

  return true;
}

template <typename PubSubClientPolicy>
void MqttClient<PubSubClientPolicy>::disconnect() {
  if (!mqttClient.connected()) {
    connected = false;
    return;
  }

  LOG_INFO("MQTT disconnecting...");
  mqttClient.disconnect();
  connected = false;
  LOG_INFO("MQTT disconnected");
}

// Note: This method updates internal connection state but doesn't perform
// actual reconnection. To reconnect, call connect() again.
template <typename PubSubClientPolicy>
bool MqttClient<PubSubClientPolicy>::reconnect() {
  if (!mqttClient.connected())
    return false;

  connected = mqttClient.connected();
  return connected;
}

template <typename PubSubClientPolicy>
void MqttClient<PubSubClientPolicy>::subscribeToConfiguredTopics() {
  if (topicTty0Tx.length() > 0) {
    LOG_INFO("Subscribing to tty0Rx: %s", topicTty0Rx.c_str());
    mqttClient.subscribe(topicTty0Rx.c_str(), MQTT_QOS_LEVEL);
    mqttClient.loop();
    delay(MQTT_SUBSCRIPTION_DELAY_MS);
    mqttClient.loop();
  }

  if (topicTty1Tx.length() > 0) {
    LOG_INFO("Subscribing to tty1Rx: %s", topicTty1Rx.c_str());
    mqttClient.subscribe(topicTty1Rx.c_str(), MQTT_QOS_LEVEL);
    mqttClient.loop();
    delay(MQTT_SUBSCRIPTION_DELAY_MS);
    mqttClient.loop();
  }
}

template <typename PubSubClientPolicy>
void MqttClient<PubSubClientPolicy>::handleConnectionStateChange(
    bool wasConnected) {
  if (wasConnected && !connected) {
    LOG_WARN("MQTT connection lost!");
    return;
  }

  if (!wasConnected && connected) {
    LOG_INFO("MQTT reconnected successfully!");
    subscribeToConfiguredTopics();
  }
}

template <typename PubSubClientPolicy>
void MqttClient<PubSubClientPolicy>::flushBuffersIfNeeded() {
  if ((millis() - tty0LastFlushMillis >= MQTT_PUBLISH_INTERVAL_MS)) {
    LOG_VERBOSE("Flushing tty0 buffer due to interval");
    tty0LastFlushMillis = millis();
    tty0Stream.flush();
  }

  if ((millis() - tty1LastFlushMillis >= MQTT_PUBLISH_INTERVAL_MS)) {
    LOG_VERBOSE("Flushing tty1 buffer due to interval");
    tty1LastFlushMillis = millis();
    tty1Stream.flush();
  }
}

template <typename PubSubClientPolicy>
void MqttClient<PubSubClientPolicy>::loop() {
  if (!mqttClient.connected())
    return;

  // Transfer pending data from web task to MQTT buffers
  while (!tty0PendingBuffer.empty()) {
    tty0Stream.append(tty0PendingBuffer.popFront());
  }

  while (!tty1PendingBuffer.empty()) {
    tty1Stream.append(tty1PendingBuffer.popFront());
  }

  const bool wasConnected = connected;

  // Process pending MQTT messages (PubSubClient requires multiple calls)
  for (uint8_t i = 0; i < MQTT_LOOP_ITERATIONS; i++) {
    mqttClient.loop();
  }

  connected = mqttClient.connected();

  handleConnectionStateChange(wasConnected);

  if (!connected)
    return;

  flushBuffersIfNeeded();
}

template <typename PubSubClientPolicy>
bool MqttClient<PubSubClientPolicy>::publishInfo(const types::string &data) {
  if (!mqttClient.connected()) {
    LOG_ERROR("MQTT publishInfo failed: mqttClient is null");
    return false;
  }

  if (topicInfo.length() == 0) {
    LOG_ERROR("MQTT publishInfo failed: topicInfo is empty");
    return false;
  }

  connected = mqttClient.connected();
  if (!connected) {
    LOG_WARN("MQTT publishInfo failed: not connected");
    return false;
  }

  LOG_DEBUG("MQTT publishing info to %s (%d bytes)", topicInfo.c_str(),
            data.length());

  bool result = mqttClient.publish(topicInfo.c_str(), (uint8_t *)data.c_str(),
                                   data.length(), false);
  if (!result) {
    LOG_ERROR("MQTT publishInfo failed! State: %d", mqttClient.state());
    connected = mqttClient.connected();
  } else {
    LOG_DEBUG("MQTT info published successfully");
  }

  return result;
}

template <typename PubSubClientPolicy>
void MqttClient<PubSubClientPolicy>::appendToTty0Buffer(
    const types::span<const uint8_t> &data) {
  // Accumulate only - main loop transfers to MQTT
  tty0PendingBuffer.append(data);
}

template <typename PubSubClientPolicy>
void MqttClient<PubSubClientPolicy>::appendToTty1Buffer(
    const types::span<const uint8_t> &data) {
  // Accumulate only - main loop transfers to MQTT
  tty1PendingBuffer.append(data);
}

template <typename PubSubClientPolicy>
void MqttClient<PubSubClientPolicy>::mqttCallback(char *topic, uint8_t *payload,
                                                  unsigned int length) {
  if (length >= MQTT_CALLBACK_BUFFER_SIZE)
    return;
  types::string_view topicStr(topic); // More efficient than String

  types::span<const uint8_t> payloadSpan(payload, length);
  if (topicStr == topicTty0Rx) {
    onTty0Callback(payloadSpan);
    return;
  } else if (topicStr == topicTty1Rx) {
    onTty1Callback(payloadSpan);
    return;
  } else {
    LOG_ERROR("MQTT callback received for unknown topic: %s", topic);
    return;
  }
}
} // namespace internal
// Explicit instantiation for production and test builds
#ifdef ESP_PLATFORM
template class internal::MqttClient<PubSubClient>;
#else
template class internal::MqttClient<PubSubClientTest>;
#endif
} // namespace jrb::wifi_serial
