#pragma once

#include "config.h"
#include "infrastructure/types.hpp"
#include <cstdint>

namespace jrb::wifi_serial {
/**
 * This namespace contains the template class definition for specific classes.
 * All template implementation details are encapsulated here to provide a clean
 * public API while maintaining zero-cost abstraction through policy-based
 * design.
 *
 * Usage Guidelines:
 * - DO NOT use types from this namespace directly in application code
 * - Use the public class type alias instead
 * - Direct template access is only intended for:
 *   1. Test files requiring explicit template instantiation
 *   2. Implementation files (.cpp) that define template methods
 */
namespace internal {
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
} // namespace internal
} // namespace jrb::wifi_serial
