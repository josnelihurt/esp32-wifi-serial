// Test policy implementation - only compiled for native tests
#include "domain/config/preferences_storage.h"
#include "domain/config/policy/preferences_storage_policy_test.h"

namespace jrb::wifi_serial {

// Static storage definition for TestStoragePolicy
std::map<std::string, std::variant<std::string, int32_t>>
    TestStoragePolicy::storage;

// Explicit template instantiation for test environment
template class PreferencesStorage<TestStoragePolicy>;

} // namespace jrb::wifi_serial