#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "infrastructure/types.hpp"

namespace jrb::wifi_serial {

/**
 * @file test_helpers.hpp
 * @brief Reusable testing utilities and matchers for firmware tests.
 *
 * This header provides common testing infrastructure inspired by circular_buffer_test patterns:
 * - Custom matchers with detailed error reporting for test assertions
 *
 * Design Philosophy:
 * - Policy-based testing pattern (uses test policy classes, not GoogleMock for dependencies)
 * - GoogleMock is used ONLY for custom matcher macros (MATCHER_P2, etc.)
 * - Test policies provide testable implementations of hardware interfaces
 * - Matchers provide clear diagnostics for test failures
 *
 * Pattern Example:
 * @code
 *   EXPECT_THAT(preferencesStorage,
 *               PreferencesStorageHasExpectedState(true,   // debugEnabled
 *                                                  false)); // bridgeEnabled
 * @endcode
 */

namespace test_helpers {

/**
 * Matcher for PreferencesStorage state verification.
 *
 * Verifies that a PreferencesStorage object has the expected state for
 * debugEnabled and tty02tty1Bridge flags.
 *
 * @param expectedDebugEnabled Expected value for debugEnabled flag
 * @param expectedBridgeEnabled Expected value for tty02tty1Bridge flag
 *
 * Usage:
 * @code
 *   EXPECT_THAT(storage, PreferencesStorageHasExpectedState(
 *       true,   // debugEnabled should be true
 *       false   // tty02tty1Bridge should be false
 *   ));
 * @endcode
 */
MATCHER_P2(PreferencesStorageHasExpectedState,
           expectedDebugEnabled, expectedBridgeEnabled,
           "PreferencesStorage should have specific state") {
  const auto &storage = arg;

  bool success = true;

  if (storage.debugEnabled != expectedDebugEnabled) {
    *result_listener << " where debugEnabled expected " << expectedDebugEnabled
                     << " but is " << storage.debugEnabled;
    success = false;
  }

  if (storage.tty02tty1Bridge != expectedBridgeEnabled) {
    *result_listener << " where tty02tty1Bridge expected " << expectedBridgeEnabled
                     << " but is " << storage.tty02tty1Bridge;
    success = false;
  }

  return success;
}

} // namespace test_helpers

} // namespace jrb::wifi_serial
