/**
 * @file special_character_handler_policy_test.cpp
 * @brief Policy instantiation file for SpecialCharacterHandler native tests.
 *
 * This file provides explicit template instantiation of SpecialCharacterHandler
 * with TestResetPolicy for use in native unit tests. It follows the pattern
 * established by preferences_storage_policy_test.cpp.
 *
 * Purpose:
 * - Instantiates SpecialCharacterHandler<TestResetPolicy> template
 * - Includes necessary implementation files for linking
 * - Ensures test policy is compiled into native test binary
 *
 * Dependencies:
 * - system_info.cpp: Required by SpecialCharacterHandler for system info display
 * - special_character_handler.cpp: Template implementation to instantiate
 *
 * Note: This file is included in all_tests.cpp BEFORE the actual test file
 * to ensure the template instantiation is available.
 */

#include "system_info.cpp"  // Include SystemInfo implementation (dependency)
#include "domain/config/special_character_handler.cpp"  // Include implementation
#include "domain/config/policy/hardware_reset_policy.h"
#include "policy/test_reset_policy.h"

namespace jrb::wifi_serial {

// Explicit template instantiation with TestResetPolicy for testing
template class internal::SpecialCharacterHandler<TestResetPolicy>;

} // namespace jrb::wifi_serial
