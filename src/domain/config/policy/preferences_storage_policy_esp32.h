#pragma once

#include "infrastructure/types.hpp"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>

namespace jrb::wifi_serial {

/**
 * @class ESP32StoragePolicy
 * @brief Storage policy implementation for ESP32 using NVS (Preferences).
 *
 * This policy wraps the ESP32 Preferences library and provides a
 * standard C++ interface, converting between Arduino String and types::string.
 *
 * Stack-based, zero heap allocation.
 */
class ESP32StoragePolicy {
private:
  Preferences prefs; // Stack-based ESP32 Preferences object

public:
  ESP32StoragePolicy() = default;
  ~ESP32StoragePolicy() = default;

  /**
   * @brief Opens the preferences storage namespace.
   * @param name Namespace name
   * @param readOnly true for read-only access
   */
  void begin(const char *name, bool readOnly) { prefs.begin(name, readOnly); }

  /**
   * @brief Closes the preferences storage.
   */
  void end() { prefs.end(); }

  /**
   * @brief Retrieves a string value from storage.
   * @param key The key to retrieve
   * @param defaultVal Default value if key doesn't exist
   * @return The stored value or default
   */
  types::string getString(const char *key, const types::string &defaultVal) {
    String result = prefs.getString(key, defaultVal.c_str());
    return types::string(result.c_str());
  }

  /**
   * @brief Retrieves an integer value from storage.
   * @param key The key to retrieve
   * @param defaultVal Default value if key doesn't exist
   * @return The stored value or default
   */
  int32_t getInt(const char *key, int32_t defaultVal) {
    return prefs.getInt(key, defaultVal);
  }

  /**
   * @brief Stores a string value.
   * @param key The key to store
   * @param value The value to store
   */
  void putString(const char *key, const types::string &value) {
    prefs.putString(key, value.c_str());
  }

  /**
   * @brief Stores an integer value.
   * @param key The key to store
   * @param value The value to store
   */
  void putInt(const char *key, int32_t value) { prefs.putInt(key, value); }

  /**
   * @brief Clears all stored preferences in the current namespace.
   */
  void clear() { prefs.clear(); }

  /**
   * @brief Serializes configuration data to JSON string using ArduinoJson.
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
    String output;
    StaticJsonDocument<1024> obj;
    obj["deviceName"] = deviceName.c_str();
    obj["mqttBroker"] = mqttBroker.c_str();
    obj["mqttPort"] = mqttPort;
    obj["mqttUser"] = mqttUser.c_str();
    obj["mqttPassword"] =
        mqttPassword.length() > 0 ? "********" : "NO_PASSWORD";
    obj["topicTty0Rx"] = topicTty0Rx.c_str();
    obj["topicTty0Tx"] = topicTty0Tx.c_str();
    obj["topicTty1Rx"] = topicTty1Rx.c_str();
    obj["topicTty1Tx"] = topicTty1Tx.c_str();
    obj["ipAddress"] = ipAddress.c_str();
    obj["macAddress"] = macAddress.c_str();
    obj["ssid"] = ssid.c_str();
    obj["mqtt"] = (mqttBroker.length() > 0 ? "connected" : "disconnected");
    obj["password"] = password.length() > 0 ? "********" : "NO_PASSWORD";
    obj["webUser"] = webUser.c_str();
    obj["webPassword"] = webPassword.length() > 0 ? "********" : "NO_PASSWORD";
    obj["debugEnabled"] = debugEnabled;
    obj["tty02tty1Bridge"] = tty02tty1Bridge;
    serializeJsonPretty(obj, output);
    return types::string(output.c_str());
  }
};

} // namespace jrb::wifi_serial
