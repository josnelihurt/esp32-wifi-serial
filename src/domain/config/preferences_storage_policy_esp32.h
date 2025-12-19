#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <string>

namespace jrb::wifi_serial {

/**
 * @class ESP32StoragePolicy
 * @brief Storage policy implementation for ESP32 using NVS (Preferences).
 *
 * This policy wraps the ESP32 Preferences library and provides a
 * standard C++ interface, converting between Arduino String and std::string.
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
  std::string getString(const char *key, const std::string &defaultVal) {
    String result = prefs.getString(key, defaultVal.c_str());
    return std::string(result.c_str());
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
  void putString(const char *key, const std::string &value) {
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
};

} // namespace jrb::wifi_serial
