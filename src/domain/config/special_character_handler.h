#pragma once
#include "domain/config/policy/hardware_reset_policy.h"
#include "domain/config/preferences_storage_policy.h"

namespace jrb::wifi_serial {
class SystemInfo;
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
template <typename ResetPolicy> class SpecialCharacterHandler {
private:
  bool specialCharacterMode;
  SystemInfo &systemInfo;
  jrb::wifi_serial::PreferencesStorage &preferencesStorage;
  ResetPolicy resetPolicy;

public:
  SpecialCharacterHandler(
      SystemInfo &systemInfo,
      jrb::wifi_serial::PreferencesStorage &preferencesStorage)
      : systemInfo(systemInfo), preferencesStorage(preferencesStorage),
        specialCharacterMode(false) {}

  bool handle(char c);
};
} // namespace internal

using SpecialCharacterHandler =
    internal::SpecialCharacterHandler<HardwareResetPolicy>;

} // namespace jrb::wifi_serial
