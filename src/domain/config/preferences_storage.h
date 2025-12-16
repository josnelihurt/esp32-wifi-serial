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

public:
  /**
   * @brief Constructor for PreferencesStorage.
   */
  PreferencesStorage() ;
  PreferencesStorage(const PreferencesStorage &) = delete;
  PreferencesStorage &operator=(const PreferencesStorage &) = delete;

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
  bool debugEnabled;
  bool tty02tty1Bridge;

  /**
   * @brief Serializes the configuration to a JSON string.
   * @return The configuration as a JSON string.
   */
  String serialize(const String &ipAddress, const String &macAddress,
                   const String &ssid);
  void save();
  void clear();

private:
  void load();
  void generateDefaultTopics();
};

} // namespace jrb::wifi_serial
