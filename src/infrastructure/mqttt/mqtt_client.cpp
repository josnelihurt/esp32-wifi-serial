#include "mqtt_client.h"
#include "config.h"
#include "domain/config/preferences_storage.h"
#include <ArduinoLog.h>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace jrb::wifi_serial {

// Constants
namespace {
constexpr size_t MQTT_CALLBACK_BUFFER_SIZE = 512;
constexpr uint8_t MQTT_QOS_LEVEL = 1;
constexpr uint16_t MQTT_KEEPALIVE_SEC = 60;
constexpr uint16_t MQTT_SOCKET_TIMEOUT_SEC = 15;
constexpr uint8_t MQTT_SUBSCRIPTION_DELAY_MS = 10;
constexpr uint8_t MQTT_LOOP_ITERATIONS = 3;
} // namespace

// Static pointer to MqttClient instance for C-style callback from PubSubClient.
// PubSubClient requires a C function pointer, so we use this to access instance
// data.
static MqttClient *g_mqttInstance{};

MqttClient::MqttClient(WiFiClient &wifiClient,
                       PreferencesStorageDefault &preferencesStorage)
    : mqttClient{std::make_unique<PubSubClient>(wifiClient)},
      wifiClient{&wifiClient}, preferencesStorage{preferencesStorage},
      connected{false}, lastReconnectAttempt{0}, onTty0Callback{nullptr},
      onTty1Callback{nullptr},
      tty0Stream{MqttFlushPolicy{*mqttClient, topicTty0Tx}, "tty0"},
      tty1Stream{MqttFlushPolicy{*mqttClient, topicTty1Tx}, "tty1"},
      tty0LastFlushMillis{0}, tty1LastFlushMillis{0} {

  mqttClient->setBufferSize(MQTT_BUFFER_SIZE);
  mqttClient->setCallback([&](char *topic, byte *payload, unsigned int length) {
    mqttCallback(topic, payload, length);
  });
  mqttClient->setKeepAlive(MQTT_KEEPALIVE_SEC);
  mqttClient->setSocketTimeout(MQTT_SOCKET_TIMEOUT_SEC);
  setTopics(preferencesStorage.topicTty0Rx, preferencesStorage.topicTty0Tx,
            preferencesStorage.topicTty1Rx, preferencesStorage.topicTty1Tx);
  g_mqttInstance = this;
}

MqttClient::~MqttClient() {
  if (g_mqttInstance == this) {
    g_mqttInstance = nullptr;
  }
  // mqttClient is automatically cleaned up by unique_ptr
}

void MqttClient::setTopics(const std::string &tty0Rx, const std::string &tty0Tx,
                           const std::string &tty1Rx,
                           const std::string &tty1Tx) {
  topicTty0Rx = tty0Rx;
  topicTty0Tx = tty0Tx;
  topicTty1Rx = tty1Rx;
  topicTty1Tx = tty1Tx;
  // Generate info topic from tty0Rx topic (replace /ttyS0/rx with /info)
  topicInfo = tty0Rx;
  size_t pos = topicInfo.find("/ttyS0/rx");
  if (pos != std::string::npos) {
    topicInfo.replace(pos, 9, "/info");
  } else if (topicInfo.size() >= 3 &&
             topicInfo.substr(topicInfo.size() - 3) == "/rx") {
    size_t lastSlash = topicInfo.rfind('/');
    if (lastSlash != std::string::npos) {
      topicInfo = topicInfo.substr(0, lastSlash) + "/info";
    }
  }
  Log.infoln("MQTT info topic set to: %s", topicInfo.c_str());
}

void MqttClient::setCallbacks(
    void (*tty0)(const nonstd::span<const uint8_t> &),
    void (*tty1)(const nonstd::span<const uint8_t> &)) {
  onTty0Callback = tty0;
  onTty1Callback = tty1;
}

bool MqttClient::connect(const char *broker, int port, const char *user,
                         const char *password) {
  mqttClient->setServer(broker, port);

  std::ostringstream oss;
  oss << "ESP32-C3-" << preferencesStorage.deviceName << "-" << std::hex
      << random(0xffff);
  std::string clientId = oss.str();

  Log.infoln("MQTT connecting to %s:%d (ClientID: %s)", broker, port,
             clientId.c_str());

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

  subscribeToConfiguredTopics();

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

// Note: This method updates internal connection state but doesn't perform
// actual reconnection. To reconnect, call connect() again.
bool MqttClient::reconnect() {
  if (!mqttClient)
    return false;

  connected = mqttClient->connected();
  return connected;
}

void MqttClient::subscribeToConfiguredTopics() {
  if (topicTty0Tx.length() > 0) {
    Log.infoln("Subscribing to tty0Rx: %s", topicTty0Rx.c_str());
    mqttClient->subscribe(topicTty0Rx.c_str(), MQTT_QOS_LEVEL);
    mqttClient->loop();
    delay(MQTT_SUBSCRIPTION_DELAY_MS);
    mqttClient->loop();
  }

  if (topicTty1Tx.length() > 0) {
    Log.infoln("Subscribing to tty1Rx: %s", topicTty1Rx.c_str());
    mqttClient->subscribe(topicTty1Rx.c_str(), MQTT_QOS_LEVEL);
    mqttClient->loop();
    delay(MQTT_SUBSCRIPTION_DELAY_MS);
    mqttClient->loop();
  }
}

void MqttClient::handleConnectionStateChange(bool wasConnected) {
  if (wasConnected && !connected) {
    Log.warningln("MQTT connection lost!");
    return;
  }

  if (!wasConnected && connected) {
    Log.infoln("MQTT reconnected successfully!");
    subscribeToConfiguredTopics();
  }
}

void MqttClient::flushBuffersIfNeeded() {
  if ((millis() - tty0LastFlushMillis >= MQTT_PUBLISH_INTERVAL_MS)) {
    Log.verboseln("Flushing tty0 buffer due to interval");
    tty0LastFlushMillis = millis();
    tty0Stream.flush();
  }

  if ((millis() - tty1LastFlushMillis >= MQTT_PUBLISH_INTERVAL_MS)) {
    Log.verboseln("Flushing tty1 buffer due to interval");
    tty1LastFlushMillis = millis();
    tty1Stream.flush();
  }
}

void MqttClient::loop() {
  if (!mqttClient)
    return;

  // Transfer pending data from web task to MQTT buffers
  if (!tty0PendingBuffer.empty()) {
    tty0Stream.append(nonstd::span<const uint8_t>(tty0PendingBuffer.data(),
                                                  tty0PendingBuffer.size()));
    tty0PendingBuffer.clear();
  }

  if (!tty1PendingBuffer.empty()) {
    tty1Stream.append(nonstd::span<const uint8_t>(tty1PendingBuffer.data(),
                                                  tty1PendingBuffer.size()));
    tty1PendingBuffer.clear();
  }

  const bool wasConnected = connected;

  // Process pending MQTT messages (PubSubClient requires multiple calls)
  for (uint8_t i = 0; i < MQTT_LOOP_ITERATIONS; i++) {
    mqttClient->loop();
  }

  connected = mqttClient->connected();

  handleConnectionStateChange(wasConnected);

  if (!connected)
    return;

  flushBuffersIfNeeded();
}

bool MqttClient::publishInfo(const std::string &data) {
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

  Log.traceln("MQTT publishing info to %s (%d bytes)", topicInfo.c_str(),
              data.length());

  bool result = mqttClient->publish(topicInfo.c_str(), (uint8_t *)data.c_str(),
                                    data.length(), false);
  if (!result) {
    Log.errorln("MQTT publishInfo failed! State: %d", mqttClient->state());
    connected = mqttClient->connected();
  } else {
    Log.traceln("MQTT info published successfully");
  }

  return result;
}

void MqttClient::appendToTty0Buffer(const nonstd::span<const uint8_t> &data) {
  // Accumulate only - main loop transfers to MQTT
  tty0PendingBuffer.insert(tty0PendingBuffer.end(), data.data(),
                           data.data() + data.size());
}

void MqttClient::appendToTty1Buffer(const nonstd::span<const uint8_t> &data) {
  // Accumulate only - main loop transfers to MQTT
  tty1PendingBuffer.insert(tty1PendingBuffer.end(), data.data(),
                           data.data() + data.size());
}

void MqttClient::mqttCallback(char *topic, byte *payload, unsigned int length) {
  if (length >= MQTT_CALLBACK_BUFFER_SIZE)
    return;
  std::string_view topicStr(topic); // More efficient than String

  nonstd::span<const uint8_t> payloadSpan(payload, length);
  if (topicStr == topicTty0Rx) {
    onTty0Callback(payloadSpan);
    return;
  } else if (topicStr == topicTty1Rx) {
    onTty1Callback(payloadSpan);
    return;
  } else {
    Log.errorln("MQTT callback received for unknown topic: %s", topic);
    return;
  }
}

} // namespace jrb::wifi_serial
