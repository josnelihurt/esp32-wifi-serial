#pragma once

#include "config.h"
#include "infrastructure/types.hpp"
#include <cstdint>

namespace jrb::wifi_serial {

/**
 * @class PreferencesStorage
 * @brief A template-based class to manage device configuration preferences.
 * @tparam StoragePolicy The storage backend policy (ESP32 or Test)
 *
 * This design uses policy-based design for zero-cost abstraction:
 * - No virtual functions, no vtables
 * - Stack-based storage policy
 * - Compile-time polymorphism
 * - Platform-independent header using only standard C++ types
 */
template <typename StoragePolicy> class PreferencesStorage final {
private:
  StoragePolicy storage; // Stack-based storage backend

public:
  /**
   * @brief Constructor for PreferencesStorage.
   * Automatically loads preferences on construction.
   */
  PreferencesStorage();
  PreferencesStorage(const PreferencesStorage &) = delete;
  PreferencesStorage &operator=(const PreferencesStorage &) = delete;

  ~PreferencesStorage() = default;

  // Configuration fields - using standard C++ types
  types::string deviceName;
  int32_t baudRateTty1;
  types::string mqttBroker;
  int32_t mqttPort;
  types::string mqttUser;
  types::string mqttPassword;
  types::string topicTty0Rx;
  types::string topicTty0Tx;
  types::string topicTty1Rx;
  types::string topicTty1Tx;
  types::string ssid;
  types::string password;
  types::string webUser;
  types::string webPassword;
  bool debugEnabled;
  bool tty02tty1Bridge;

  /**
   * @brief Serializes the configuration to a JSON string.
   * @param ipAddress The device IP address
   * @param macAddress The device MAC address
   * @param ssid The connected SSID
   * @return The configuration as a JSON string.
   */
  types::string serialize(const types::string &ipAddress,
                          const types::string &macAddress,
                          const types::string &ssid) const;

  /**
   * @brief Saves current configuration to persistent storage.
   */
  void save();

  /**
   * @brief Clears all stored preferences and resets to defaults.
   */
  void clear();

private:
  /**
   * @brief Loads configuration from persistent storage.
   */
  void load();

  /**
   * @brief Generates default MQTT topics based on device name.
   */
  void generateDefaultTopics();
};

// Forward declarations of policy implementations
class ESP32StoragePolicy;
class TestStoragePolicy;

// Type aliases for convenience
using PreferencesStorageESP32 = PreferencesStorage<ESP32StoragePolicy>;
using PreferencesStorageTest = PreferencesStorage<TestStoragePolicy>;

// PreferencesStorageDefault is defined in preferences_storage_policy.h
// based on ESP_PLATFORM

} // namespace jrb::wifi_serial
