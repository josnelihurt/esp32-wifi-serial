#include "mqtt_client.h"
#include "config.h"
#include <ArduinoLog.h>
#include <cstring>

namespace jrb::wifi_serial {

// Static pointer to MqttClient instance for C-style callback from PubSubClient.
// PubSubClient requires a C function pointer, so we use this to access instance
// data.
static MqttClient *g_mqttInstance{};

MqttClient::MqttClient(WiFiClient &wifiClient)
    : mqttClient{new PubSubClient(wifiClient)}, wifiClient{&wifiClient},
      deviceName{DEFAULT_DEVICE_NAME}, connected{false},
      lastReconnectAttempt{0}, onTty0Callback{nullptr}, onTty1Callback{nullptr},
      tty0Buffer{0}, tty1Buffer{0}, tty0LastFlushMillis{0},
      tty1LastFlushMillis{0} {
  tty0Buffer.reserve(MQTT_BUFFER_SIZE);
  tty1Buffer.reserve(MQTT_BUFFER_SIZE);
  tty0LastFlushMillis = millis();
  tty1LastFlushMillis = millis();
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
  if (!mqttClient)
    return;
  delete mqttClient;
}

void MqttClient::setDeviceName(const String &name) { deviceName = name; }

void MqttClient::setTopics(const String &tty0Rx, const String &tty0Tx,
                           const String &tty1Rx, const String &tty1Tx) {
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

void MqttClient::setCallbacks(void (*tty0)(const char *, unsigned int),
                              void (*tty1)(const char *, unsigned int)) {
  onTty0Callback = tty0;
  onTty1Callback = tty1;
}

bool MqttClient::connect(const char *broker, int port, const char *user,
                         const char *password) {
  mqttClient->setServer(broker, port);

  String clientId =
      "ESP32-C3-" + deviceName + "-" + String(random(0xffff), HEX);

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
  if (!mqttClient)
    return false;
  connected = mqttClient->connected();
  return connected;
}

void MqttClient::loop() {
  if (!mqttClient)
    return;

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
  if (connected) {
    if (tty0LastFlushMillis < millis() - MQTT_PUBLISH_INTERVAL_MS && tty0Buffer.size() > 0) {
      Log.infoln("Flushing tty0 buffer due to interval");
      tty0LastFlushMillis = millis();
      flushTty0Buffer();
    }
    if (tty1LastFlushMillis < millis() - MQTT_PUBLISH_INTERVAL_MS && tty1Buffer.size() > 0) {
      Log.infoln("Flushing tty1 buffer due to interval");
      tty1LastFlushMillis = millis();
      flushTty1Buffer();
    }
  }
}

bool MqttClient::publishInfo(const String &data) {
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

void MqttClient::appendToBufferWithFlush(std::vector<char> &buffer,
                                         const std::vector<char> &data,
                                         std::function<void()> flushFunction,
                                         const char* const flushFunctionName) {
  if (data.empty()) {
    return;
  }
  Log.verboseln("%s: Appending %d bytes to buffer", flushFunctionName,
             data.size());
  if (data.size() + buffer.size() > MQTT_BUFFER_SIZE) {
    Log.verboseln("%s: Buffer size exceeded, flushing buffer", flushFunctionName);
    flushFunction();
  }
  buffer.insert(buffer.end(), data.begin(), data.end());
  if(buffer.back() == '\n') {
    Log.verboseln("%s: Flushing buffer due to newline", flushFunctionName);
    flushFunction();
  }
}

void MqttClient::appendToTty0Buffer(const std::vector<char> &data) {
  appendToBufferWithFlush(tty0Buffer, data, [&]() { flushTty0Buffer(); });
}

void MqttClient::appendToTty1Buffer(const std::vector<char> &data) {
  appendToBufferWithFlush(tty1Buffer, data, [&]() { flushTty1Buffer(); });
}

void MqttClient::flushBuffer(std::vector<char> &buffer,
                             const String &topic, unsigned long &lastFlushMillis,
                             const char *flushFunctionName) {
  if (buffer.size() == 0) {
    return;
  }
  if (!connected) {
    Log.warningln("MQTT connection lost, dropping buffer for %s",
                  flushFunctionName);
    buffer.clear();
    lastFlushMillis = millis();
    return;
  }
  mqttClient->publish(topic.c_str(), (uint8_t *)buffer.data(), buffer.size(),
                      false);
  buffer.clear();
  lastFlushMillis = millis();
  Log.infoln("%s: Buffer flushed for %s", flushFunctionName, topic.c_str());
}

void MqttClient::flushTty0Buffer() {
  flushBuffer(tty0Buffer, topicTty0Rx, tty0LastFlushMillis);
}

void MqttClient::flushTty1Buffer() {
  flushBuffer(tty1Buffer, topicTty1Rx, tty1LastFlushMillis);
}

void MqttClient::mqttCallback(char *topic, byte *payload, unsigned int length) {
  if (!g_mqttInstance)
    return;
  if (length >= 512)
    return;

  static char buffer[512];
  memcpy(buffer, payload, length);
  buffer[length] = '\0';

  String topicStr = String(topic);

  if ((topicStr == g_mqttInstance->topicTty0Tx ||
       strcmp(topic, g_mqttInstance->topicTty0Tx.c_str()) == 0)) {
    if (g_mqttInstance->onTty0Callback) {
      g_mqttInstance->onTty0Callback(buffer, length);
    }
    return;
  }

  if ((topicStr == g_mqttInstance->topicTty1Tx ||
       strcmp(topic, g_mqttInstance->topicTty1Tx.c_str()) == 0)) {
    if (g_mqttInstance->onTty1Callback) {
      g_mqttInstance->onTty1Callback(buffer, length);
    }
    return;
  }
}

} // namespace jrb::wifi_serial
