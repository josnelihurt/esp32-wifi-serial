/**
 * @file special_character_handler_test.cpp
 * @brief Policy-based tests for SpecialCharacterHandler
 *
 * This test file demonstrates the policy-based testing pattern used throughout
 * the firmware test suite (inspired by circular_buffer_test.cpp):
 * - Uses TestResetPolicy instead of mocking for hardware abstraction
 * - Creates real PreferencesStorage and SystemInfo objects
 * - Tests observable behavior and state changes
 *
 * Testing Limitations:
 * - Cannot verify internal resetPolicy state (private member)
 * - Cannot verify logging output (would require mock logger)
 * - Cannot verify exact delay timing (relies on policy behavior)
 *
 * Command Constants (defined in src/config.h):
 * - CMD_PREFIX = 0x19 (Ctrl+Y) - Activates special character mode
 * - CMD_INFO = 'i' - Displays system information
 * - CMD_DEBUG = 'd' - Toggles debug mode
 * - CMD_TTY0_TO_TTY1_BRIDGE = 'b' - Toggles TTY bridge
 * - CMD_RESET = 0x0E - Resets device (with cancellable countdown)
 */

#include "domain/config/special_character_handler.h"
#include "domain/config/preferences_storage.h"
#include "domain/config/policy/preferences_storage_policy_test.h"
#include "policy/test_reset_policy.h"
#include "system_info.h"
#include "config.h"  // For CMD_* constants
#include <gtest/gtest.h>
#include "test_helpers.hpp"

namespace jrb::wifi_serial {
namespace {

using test_helpers::PreferencesStorageHasExpectedState;

/**
 * Test fixture for SpecialCharacterHandler with TestResetPolicy.
 *
 * Note: Member initialization order matters - systemInfo depends on
 * preferencesStorage and otaEnabled, handler depends on both.
 */
class SpecialCharacterHandlerTest : public testing::Test {
protected:
  bool otaEnabled;
  PreferencesStorage preferencesStorage;
  SystemInfo systemInfo;
  internal::SpecialCharacterHandler<TestResetPolicy> handler;

  SpecialCharacterHandlerTest()
      : otaEnabled(false),
        preferencesStorage(),  // Default constructor initializes with defaults
        systemInfo(preferencesStorage, otaEnabled),
        handler(systemInfo, preferencesStorage) {}

  // Note: No SetUp() needed - member initialization in constructor is sufficient
  // PreferencesStorage default constructor already sets debugEnabled=false,
  // tty02tty1Bridge=false

  void TearDown() override {}  // Intentionally empty
};

// ============================================================================
// CMD_PREFIX Tests
// ============================================================================

TEST_F(SpecialCharacterHandlerTest, CmdPrefixActivatesSpecialMode) {
  // CMD_PREFIX (Ctrl+Y) should activate special mode and return true
  EXPECT_TRUE(handler.handle(CMD_PREFIX));
}

TEST_F(SpecialCharacterHandlerTest, CmdPrefixMultipleCallsStillReturnsTrue) {
  // Calling CMD_PREFIX multiple times should keep returning true
  EXPECT_TRUE(handler.handle(CMD_PREFIX));
  EXPECT_TRUE(handler.handle(CMD_PREFIX));
}

// ============================================================================
// CMD_INFO Tests
// ============================================================================

TEST_F(SpecialCharacterHandlerTest, CmdInfoRequiresPrefixFirst) {
  // CMD_INFO without prefix should return false (not in special mode)
  EXPECT_FALSE(handler.handle(CMD_INFO));
}

TEST_F(SpecialCharacterHandlerTest, CmdInfoAfterPrefixReturnsFalse) {
  // Activate special mode first
  EXPECT_TRUE(handler.handle(CMD_PREFIX));

  // CMD_INFO executes and deactivates special mode, returning false
  // (Commands return false after execution to indicate mode exit)
  EXPECT_FALSE(handler.handle(CMD_INFO));
}

// ============================================================================
// CMD_DEBUG Tests
// ============================================================================

TEST_F(SpecialCharacterHandlerTest, CmdDebugTogglesDebugMode) {
  // Initial state: debugEnabled=false (from default constructor)

  // Activate special mode and toggle debug
  handler.handle(CMD_PREFIX);
  handler.handle(CMD_DEBUG);

  // Verify debug is now enabled
  EXPECT_THAT(preferencesStorage,
              PreferencesStorageHasExpectedState(true /* debugEnabled */,
                                                 false /* bridgeEnabled */));
}

TEST_F(SpecialCharacterHandlerTest, CmdDebugTogglesDebugModeMultipleTimes) {
  // Initial state: debugEnabled=false

  // First toggle: false -> true
  handler.handle(CMD_PREFIX);
  handler.handle(CMD_DEBUG);
  EXPECT_THAT(preferencesStorage,
              PreferencesStorageHasExpectedState(true, false));

  // Second toggle: true -> false
  handler.handle(CMD_PREFIX);
  handler.handle(CMD_DEBUG);
  EXPECT_THAT(preferencesStorage,
              PreferencesStorageHasExpectedState(false, false));
}

// ============================================================================
// CMD_TTY0_TO_TTY1_BRIDGE Tests
// ============================================================================

TEST_F(SpecialCharacterHandlerTest, CmdBridgeTogglesTTYBridge) {
  // Initial state: tty02tty1Bridge=false (from default constructor)

  handler.handle(CMD_PREFIX);
  handler.handle(CMD_TTY0_TO_TTY1_BRIDGE);

  // Verify bridge is now enabled
  EXPECT_THAT(preferencesStorage,
              PreferencesStorageHasExpectedState(false /* debugEnabled */,
                                                 true /* bridgeEnabled */));
}

TEST_F(SpecialCharacterHandlerTest, CmdBridgeTogglesTTYBridgeMultipleTimes) {
  // Initial state: tty02tty1Bridge=false

  // First toggle: false -> true
  handler.handle(CMD_PREFIX);
  handler.handle(CMD_TTY0_TO_TTY1_BRIDGE);
  EXPECT_THAT(preferencesStorage,
              PreferencesStorageHasExpectedState(false, true));

  // Second toggle: true -> false
  handler.handle(CMD_PREFIX);
  handler.handle(CMD_TTY0_TO_TTY1_BRIDGE);
  EXPECT_THAT(preferencesStorage,
              PreferencesStorageHasExpectedState(false, false));
}

// ============================================================================
// CMD_RESET Tests
// ============================================================================

TEST_F(SpecialCharacterHandlerTest, CmdResetExecutesWhenNoSerialData) {
  // Note: We test observable behavior since resetPolicy is private
  // The reset command completes countdown and returns false when finished
  handler.handle(CMD_PREFIX);
  EXPECT_FALSE(handler.handle(CMD_RESET));

  // Limitation: Cannot verify resetDevice() was called or delay timing
  // due to private resetPolicy member in SpecialCharacterHandler
}

// Note: Cannot fully test reset cancellation behavior because:
// 1. resetPolicy member is private in SpecialCharacterHandler
// 2. setSerialDataAvailable() cannot be called during the countdown
// 3. Would require either: exposing policy for testing OR using dependency injection
// This is a known testing limitation of the current design.

// ============================================================================
// Edge Cases - Unknown Character
// ============================================================================

TEST_F(SpecialCharacterHandlerTest, UnknownCharacterWithoutPrefixReturnsFalse) {
  // Characters that aren't CMD_PREFIX should return false in normal mode
  EXPECT_FALSE(handler.handle('X'));
  EXPECT_FALSE(handler.handle('?'));
  EXPECT_FALSE(handler.handle(123));
}

TEST_F(SpecialCharacterHandlerTest, UnknownCharacterAfterPrefixReturnsFalse) {
  // Unknown commands after prefix should return false and exit special mode
  handler.handle(CMD_PREFIX);
  EXPECT_FALSE(handler.handle('X'));

  // Special mode should be reset, so another unknown char returns false
  handler.handle(CMD_PREFIX);
  EXPECT_FALSE(handler.handle('?'));
}

// ============================================================================
// Edge Cases - Special Mode State Machine
// ============================================================================

TEST_F(SpecialCharacterHandlerTest, SpecialModeResetsAfterCommand) {
  // Activate special mode
  EXPECT_TRUE(handler.handle(CMD_PREFIX));

  // Execute a command (deactivates special mode)
  handler.handle(CMD_DEBUG);

  // Next character should not be in special mode
  // CMD_DEBUG without prefix should be ignored
  EXPECT_FALSE(handler.handle(CMD_DEBUG));
}

TEST_F(SpecialCharacterHandlerTest, NormalCharactersIgnoredWhenNotInSpecialMode) {
  // Initial state: both flags false

  // Send command characters without prefix - should be ignored
  EXPECT_FALSE(handler.handle(CMD_DEBUG));
  EXPECT_FALSE(handler.handle(CMD_TTY0_TO_TTY1_BRIDGE));

  // State should not change
  EXPECT_THAT(preferencesStorage,
              PreferencesStorageHasExpectedState(false, false));
}

} // namespace
} // namespace jrb::wifi_serial
