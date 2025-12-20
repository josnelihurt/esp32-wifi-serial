
#include "preferences_storage_policy_esp32.h"
#include "preferences_storage.h"

namespace jrb::wifi_serial {

template class PreferencesStorage<ESP32StoragePolicy>;

} // namespace jrb::wifi_serial