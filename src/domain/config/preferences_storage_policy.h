#pragma once

/**
 * @file preferences_storage_policy.h
 * @brief Central header for storage policy selection based on platform.
 *
 * This header consolidates all platform-specific #ifdef directives for
 * PreferencesStorage policies, keeping the main implementation clean.
 *
 * - ESP32: Uses ESP32StoragePolicy with NVS and ArduinoJson
 * - Test/Native: Uses TestStoragePolicy with std::map and std::ostringstream
 */

#ifdef ESP_PLATFORM
// ESP32 Platform - Use NVS and ArduinoJson
#include "domain/config/policy/preferences_storage_policy_esp32.h"
#else
// Test/Native Platform - Use in-memory storage and ostringstream
#include "domain/config/policy/preferences_storage_policy_test.h"
#endif

namespace jrb::wifi_serial {

// Forward declaration of the template class
namespace internal {
template <typename StoragePolicy> class PreferencesStorage;
}

// Type aliases for convenience
#ifdef ESP_PLATFORM
using PreferencesStorage = internal::PreferencesStorage<ESP32StoragePolicy>;
#else
using PreferencesStorage = internal::PreferencesStorage<TestStoragePolicy>;
#endif

} // namespace jrb::wifi_serial
