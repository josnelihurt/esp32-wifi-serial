#pragma once

#include "config.h"
#include <cstdint>
#include <string>

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
template <typename StoragePolicy>
class PreferencesStorage final {
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
  std::string deviceName;
  int32_t baudRateTty1;
  std::string mqttBroker;
  int32_t mqttPort;
  std::string mqttUser;
  std::string mqttPassword;
  std::string topicTty0Rx;
  std::string topicTty0Tx;
  std::string topicTty1Rx;
  std::string topicTty1Tx;
  std::string ssid;
  std::string password;
  std::string webUser;
  std::string webPassword;
  bool debugEnabled;
  bool tty02tty1Bridge;

  /**
   * @brief Serializes the configuration to a JSON string.
   * @param ipAddress The device IP address
   * @param macAddress The device MAC address
   * @param ssid The connected SSID
   * @return The configuration as a JSON string.
   */
  std::string serialize(const std::string &ipAddress,
                        const std::string &macAddress,
                        const std::string &ssid) const;

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

// For backwards compatibility in production code
#ifdef ESP_PLATFORM
using PreferencesStorageDefault = PreferencesStorageESP32;
#else
using PreferencesStorageDefault = PreferencesStorageTest;
#endif

} // namespace jrb::wifi_serial
