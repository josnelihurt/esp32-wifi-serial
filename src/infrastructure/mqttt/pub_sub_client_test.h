/**
 * @file pub_sub_client_test.h
 * @brief Mock PubSubClient implementation for unit testing MqttClient.
 *
 * This class provides a zero-cost abstraction for testing MQTT functionality
 * without requiring actual MQTT broker connectivity. It implements the same
 * interface as the real PubSubClient library.
 *
 * Design follows the policy-based design pattern used elsewhere in the codebase
 * (see preferences_storage_policy_test.h for similar pattern).
 */

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace jrb::wifi_serial {

/**
 * @class PubSubClientTest
 * @brief Mock implementation of PubSubClient for testing.
 *
 * Provides a minimal interface matching PubSubClient library requirements.
 * Tracks state and method calls for test verification.
 */
class PubSubClientTest {
private:
  std::string server_;
  int port_{0};
  bool connected_{false};
  int state_{0}; // MQTT state code
  size_t bufferSize_{256};
  uint16_t keepAlive_{15};
  uint16_t socketTimeout_{15};
  std::function<void(char *, uint8_t *, unsigned int)> callback_;

  // Test tracking
  std::vector<std::string> subscribedTopics_;
  std::vector<std::string> publishedTopics_;

public:
  PubSubClientTest() = default;
  ~PubSubClientTest() = default;

  // Core PubSubClient interface

  /**
   * @brief Set the MQTT broker server and port.
   */
  void setServer(const char *domain, int port) {
    server_ = domain;
    port_ = port;
  }

  /**
   * @brief Set the message callback function.
   */
  void setCallback(std::function<void(char *, uint8_t *, unsigned int)> callback) {
    callback_ = callback;
  }

  /**
   * @brief Set the buffer size for messages.
   */
  void setBufferSize(size_t size) { bufferSize_ = size; }

  /**
   * @brief Set the keep-alive interval.
   */
  void setKeepAlive(uint16_t keepAlive) { keepAlive_ = keepAlive; }

  /**
   * @brief Set the socket timeout.
   */
  void setSocketTimeout(uint16_t timeout) { socketTimeout_ = timeout; }

  /**
   * @brief Connect to MQTT broker without credentials.
   */
  bool connect(const char *clientId) {
    (void)clientId;
    connected_ = true;
    state_ = 0; // MQTT_CONNECTED
    return true;
  }

  /**
   * @brief Connect to MQTT broker with credentials.
   */
  bool connect(const char *clientId, const char *user, const char *password) {
    (void)clientId;
    (void)user;
    (void)password;
    connected_ = true;
    state_ = 0; // MQTT_CONNECTED
    return true;
  }

  /**
   * @brief Disconnect from MQTT broker.
   */
  void disconnect() {
    connected_ = false;
    state_ = -1; // MQTT_DISCONNECTED
  }

  /**
   * @brief Check if connected to broker.
   */
  bool connected() const { return connected_; }

  /**
   * @brief Get current MQTT state code.
   */
  int state() const { return state_; }

  /**
   * @brief Subscribe to a topic.
   */
  bool subscribe(const char *topic, uint8_t qos) {
    (void)qos;
    if (connected_) {
      subscribedTopics_.push_back(topic);
      return true;
    }
    return false;
  }

  /**
   * @brief Process incoming messages (must be called regularly).
   */
  void loop() {
    // In test mode, this is a no-op unless we inject messages
  }

  /**
   * @brief Publish a message to a topic (3-parameter version).
   */
  bool publish(const char *topic, const uint8_t *payload, unsigned int length) {
    return publish(topic, payload, length, false);
  }

  /**
   * @brief Publish a message to a topic (4-parameter version with retained flag).
   */
  bool publish(const char *topic, const uint8_t *payload, unsigned int length,
               bool retained) {
    (void)payload;
    (void)length;
    (void)retained;
    if (connected_) {
      publishedTopics_.push_back(topic);
      return true;
    }
    return false;
  }

  // Test-specific methods for verification

  /**
   * @brief Get list of subscribed topics (test helper).
   */
  const std::vector<std::string> &getSubscribedTopics() const {
    return subscribedTopics_;
  }

  /**
   * @brief Get list of published topics (test helper).
   */
  const std::vector<std::string> &getPublishedTopics() const {
    return publishedTopics_;
  }

  /**
   * @brief Reset mock state (test helper).
   */
  void reset() {
    connected_ = false;
    state_ = -1;
    subscribedTopics_.clear();
    publishedTopics_.clear();
  }

  /**
   * @brief Simulate receiving a message (test helper).
   */
  void simulateMessage(const char *topic, const uint8_t *payload,
                       unsigned int length) {
    if (callback_ && connected_) {
      // PubSubClient passes mutable pointers
      char *topicMutable = const_cast<char *>(topic);
      uint8_t *payloadMutable = const_cast<uint8_t *>(payload);
      callback_(topicMutable, payloadMutable, length);
    }
  }

  /**
   * @brief Set connection state for testing (test helper).
   */
  void setConnected(bool connected) {
    connected_ = connected;
    state_ = connected ? 0 : -1;
  }
};

} // namespace jrb::wifi_serial
