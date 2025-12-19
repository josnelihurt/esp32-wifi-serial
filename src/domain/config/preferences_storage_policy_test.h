#pragma once

#include <cstdint>
#include <map>
#include <string>
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
  static std::map<std::string, std::variant<std::string, int32_t>> storage;
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
  std::string getString(const char *key, const std::string &defaultVal) {
    auto it = storage.find(key);
    if (it != storage.end()) {
      if (std::holds_alternative<std::string>(it->second)) {
        return std::get<std::string>(it->second);
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
  void putString(const char *key, const std::string &value) {
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
  bool hasKey(const std::string &key) const {
    return storage.find(key) != storage.end();
  }

  /**
   * @brief Resets the storage to empty state (test helper).
   */
  void reset() { storage.clear(); }
};

} // namespace jrb::wifi_serial
