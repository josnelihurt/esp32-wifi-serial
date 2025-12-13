#pragma once

#include "config.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

namespace jrb::wifi_serial {

/**
 * @brief MQTT client wrapper for ESP32 WiFi serial communication.
 */
class MqttClient final {
public:
  /** Constructs a new Mqtt Client object.
   * @param wifiClient Reference to the underlying WiFi client used by
   * PubSubClient.
   */
  MqttClient(WiFiClient &wifiClient);
  ~MqttClient();

  /** Sets device name used in MQTT topics and info messages. */
  void setDeviceName(const String &name);

  /** Configures the four serial topic pairs (Rx/Tx for tty0 and tty1). */
  void setTopics(const String &tty0Rx, const String &tty0Tx,
                 const String &tty1Rx, const String &tty1Tx);

  /** Registers callbacks invoked when messages are received on the Rx topics.
   */
  void setCallbacks(void (*tty0)(const char *, unsigned int),
                    void (*tty1)(const char *, unsigned int));

  /** Connects to an MQTT broker.
   * @param broker Hostname or IP address of the broker.
   * @param port TCP port.
   * @param user Optional username.
   * @param password Optional password.
   */
  bool connect(const char *broker, int port, const char *user = nullptr,
               const char *password = nullptr);

  /** Disconnects from the broker. */
  void disconnect();

  /** Attempts to reconnect if connection is lost. */
  bool reconnect();

  /** Processes incoming MQTT messages and handles keep‑alive. */
  void loop();

  /** Publishes raw data to tty0 topic.
   * @param data Pointer to payload.
   * @param length Length of payload.
   */
  bool publishTty0(const char *data, unsigned int length);
  /** Publishes raw data to tty1 topic. */
  bool publishTty1(const char *data, unsigned int length);
  /** Convenience overload: publishes String to tty0. */
  bool publishTty0(const String &data);
  /** Convenience overload: publishes String to tty1. */
  bool publishTty1(const String &data);
  /** Publishes informational payload to the info topic. */
  bool publishInfo(const String &data);

  /** Queries connection state. */
  bool isConnected() const { return connected; }
  /** Updates internal connection flag (used by callbacks). */
  void setConnected(bool state) { connected = state; }
  /** Access underlying PubSubClient instance. */
  PubSubClient *getMqttClient() const { return mqttClient; }

  /** Buffer incoming data until the broker is ready.
   * @param portIndex 0 for tty0, 1 for tty1.
   */
  void appendToBuffer(int portIndex, const char *data, unsigned int length);
  /** Flushes buffered data for a specific port. */
  void flushBuffer(int portIndex);
  /** Determines if the buffer should be flushed (size threshold). */
  bool shouldFlushBuffer(int portIndex) const;

private:
  PubSubClient *mqttClient; /**< MQTT client instance. */
  WiFiClient *wifiClient;   /**< Underlying WiFi client reference. */
  String deviceName;        /**< Human‑readable device identifier. */
  String topicTty0Rx, topicTty0Tx;
  String topicTty1Rx, topicTty1Tx;
  String topicInfo; /**< Topic for informational messages. */
  bool connected;   /**< Connection state flag. */
  unsigned long
      lastReconnectAttempt; /**< Timestamp of the last reconnect attempt. */

  void (*onTty0Callback)(const char *,
                         unsigned int); /**< Rx callback for tty0. */
  void (*onTty1Callback)(const char *,
                         unsigned int); /**< Rx callback for tty1. */

  /** Buffer per port to reduce heap fragmentation. */
  char mqttPublishBuffer[2][MQTT_PUBLISH_BUFFER_SIZE];
  int mqttPublishBufferLen[2];      /**< Current length in each buffer. */
  unsigned long lastMqttPublish[2]; /**< Last publish time per port. */

  static void mqttCallback(char *topic, byte *payload, unsigned int length);
  bool publish(int portIndex, const char *data, unsigned int length);
};

} // namespace jrb::wifi_serial
