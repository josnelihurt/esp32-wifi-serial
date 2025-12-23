
#include "preferences_storage_policy_esp32.h"
#include "../preferences_storage.cpp" // Include implementation for template instantiation

namespace jrb::wifi_serial {

template class internal::PreferencesStorage<ESP32StoragePolicy>;

} // namespace jrb::wifi_serial