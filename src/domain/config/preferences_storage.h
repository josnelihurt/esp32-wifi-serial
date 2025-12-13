#pragma once

#include "config.h"
#include <Arduino.h>
#include <Preferences.h>

namespace jrb::wifi_serial {

/**
 * @class PreferencesStorage
 * @brief A class to manage device configuration preferences using the Arduino
 * Preferences library.
 */
class PreferencesStorage final {
private:
  Preferences preferences;
  /**
   * @brief Generates default MQTT topics.
   */
  void generateDefaultTopics();

public:
  /**
   * @brief Constructor for PreferencesStorage.
   */
  PreferencesStorage();
  ~PreferencesStorage() = default;
  String deviceName;
  int baudRateTty1;
  String mqttBroker;
  int mqttPort;
  String mqttUser;
  String mqttPassword;
  String topicTty0Rx;
  String topicTty0Tx;
  String topicTty1Rx;
  String topicTty1Tx;
  String ssid;
  String password;
  String webUser;
  String webPassword;

  /**
   * @brief Serializes the configuration to a JSON string.
   * @return The configuration as a JSON string.
   */
  String serialize(const String &ipAddress, const String &macAddress,
                   const String &ssid);
  /**
   * @brief Loads configuration from preferences storage.
   */
  void load();

  /**
   * @brief Saves current configuration to preferences storage.
   */
  void save();

  /**
   * @brief Clears all configuration in the preferences storage.
   */
  void clear();
};

} // namespace jrb::wifi_serial
