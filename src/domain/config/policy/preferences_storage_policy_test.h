#pragma once

#include "infrastructure/types.hpp"
#include <cstdint>
#include <map>
#include <sstream>
#include <variant>

namespace jrb::wifi_serial {

/**
 * @class TestStoragePolicy
 * @brief Storage policy implementation for testing using in-memory storage.
 *
 * This policy provides a mock storage backend for unit testing.
 * Uses std::map for in-memory key-value storage.
 *
 * Stack-based, deterministic behavior for testing.
 */
class TestStoragePolicy {
private:
  // Static storage to simulate persistent storage across instances
  static std::map<types::string, std::variant<types::string, int32_t>> storage;
  bool isReadOnly = false;

public:
  TestStoragePolicy() = default;
  ~TestStoragePolicy() = default;

  /**
   * @brief Opens the preferences storage namespace (no-op for testing).
   * @param name Namespace name (ignored in test)
   * @param readOnly true for read-only access
   */
  void begin(const char *name, bool readOnly) {
    (void)name; // Unused in test implementation
    isReadOnly = readOnly;
  }

  /**
   * @brief Closes the preferences storage (no-op for testing).
   */
  void end() {
    // No action needed for in-memory storage
  }

  /**
   * @brief Retrieves a string value from storage.
   * @param key The key to retrieve
   * @param defaultVal Default value if key doesn't exist
   * @return The stored value or default
   */
  types::string getString(const char *key, const types::string &defaultVal) {
    auto it = storage.find(key);
    if (it != storage.end()) {
      if (std::holds_alternative<types::string>(it->second)) {
        return std::get<types::string>(it->second);
      }
    }
    return defaultVal;
  }

  /**
   * @brief Retrieves an integer value from storage.
   * @param key The key to retrieve
   * @param defaultVal Default value if key doesn't exist
   * @return The stored value or default
   */
  int32_t getInt(const char *key, int32_t defaultVal) {
    auto it = storage.find(key);
    if (it != storage.end()) {
      if (std::holds_alternative<int32_t>(it->second)) {
        return std::get<int32_t>(it->second);
      }
    }
    return defaultVal;
  }

  /**
   * @brief Stores a string value.
   * @param key The key to store
   * @param value The value to store
   */
  void putString(const char *key, const types::string &value) {
    if (!isReadOnly) {
      storage[key] = value;
    }
  }

  /**
   * @brief Stores an integer value.
   * @param key The key to store
   * @param value The value to store
   */
  void putInt(const char *key, int32_t value) {
    if (!isReadOnly) {
      storage[key] = value;
    }
  }

  /**
   * @brief Clears all stored preferences.
   */
  void clear() {
    if (!isReadOnly) {
      storage.clear();
    }
  }

  // Test-specific methods for verification

  /**
   * @brief Gets the number of stored keys (test helper).
   * @return Number of keys in storage
   */
  size_t size() const { return storage.size(); }

  /**
   * @brief Checks if a key exists (test helper).
   * @param key The key to check
   * @return true if key exists
   */
  bool hasKey(const types::string &key) const {
    return storage.find(key) != storage.end();
  }

  /**
   * @brief Resets the storage to empty state (test helper).
   */
  void reset() { storage.clear(); }

  /**
   * @brief Serializes configuration data to JSON string using
   * std::ostringstream.
   */
  types::string serializeJson(
      const types::string &deviceName, const types::string &mqttBroker,
      int32_t mqttPort, const types::string &mqttUser,
      const types::string &mqttPassword, const types::string &topicTty0Rx,
      const types::string &topicTty0Tx, const types::string &topicTty1Rx,
      const types::string &topicTty1Tx, const types::string &ipAddress,
      const types::string &macAddress, const types::string &ssid,
      const types::string &password, const types::string &webUser,
      const types::string &webPassword, bool debugEnabled,
      bool tty02tty1Bridge) const {
    std::ostringstream oss;
    oss << "{\n"
        << "  \"deviceName\": \"" << deviceName << "\",\n"
        << "  \"mqttBroker\": \"" << mqttBroker << "\",\n"
        << "  \"mqttPort\": " << mqttPort << ",\n"
        << "  \"mqttUser\": \"" << mqttUser << "\",\n"
        << "  \"mqttPassword\": \""
        << (mqttPassword.empty() ? "NO_PASSWORD" : "********") << "\",\n"
        << "  \"topicTty0Rx\": \"" << topicTty0Rx << "\",\n"
        << "  \"topicTty0Tx\": \"" << topicTty0Tx << "\",\n"
        << "  \"topicTty1Rx\": \"" << topicTty1Rx << "\",\n"
        << "  \"topicTty1Tx\": \"" << topicTty1Tx << "\",\n"
        << "  \"ipAddress\": \"" << ipAddress << "\",\n"
        << "  \"macAddress\": \"" << macAddress << "\",\n"
        << "  \"ssid\": \"" << ssid << "\",\n"
        << "  \"mqtt\": \""
        << (mqttBroker.empty() ? "disconnected" : "connected") << "\",\n"
        << "  \"password\": \""
        << (password.empty() ? "NO_PASSWORD" : "********") << "\",\n"
        << "  \"webUser\": \"" << webUser << "\",\n"
        << "  \"webPassword\": \""
        << (webPassword.empty() ? "NO_PASSWORD" : "********") << "\",\n"
        << "  \"debugEnabled\": " << (debugEnabled ? "true" : "false") << ",\n"
        << "  \"tty02tty1Bridge\": " << (tty02tty1Bridge ? "true" : "false")
        << "\n"
        << "}";
    return oss.str();
  }
};

} // namespace jrb::wifi_serial
