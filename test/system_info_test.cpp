/**
 * @file system_info_test.cpp
 * @brief Tests for SystemInfo class functionality.
 *
 * This test file verifies the SystemInfo class which provides system
 * information display and formatting for the ESP32-C3 Serial Bridge.
 *
 * Tests verify:
 * - Special character settings string format
 * - Welcome string generation and formatting
 * - System information logging (non-crash verification only)
 *
 * Testing Limitations:
 * - Cannot verify exact log output (would require logger mocking)
 * - String assertions use substring matching, not full format validation
 * - Hardware-specific values (Serial, MAC, IP) use test stubs
 *
 * Note: This file is auto-discovered by PlatformIO and compiled separately.
 * It should NOT be included in all_tests.cpp to avoid duplicate registration.
 */

#include "system_info.h"
#include "domain/config/preferences_storage.h"
#include <gtest/gtest.h>

namespace jrb::wifi_serial {
namespace {

/**
 * Test fixture for SystemInfo.
 *
 * Creates real PreferencesStorage and SystemInfo objects for testing.
 * No SetUp() needed - constructor initialization is sufficient.
 */
class SystemInfoTest : public ::testing::Test {
protected:
  bool otaEnabled;
  PreferencesStorage preferencesStorage;
  SystemInfo systemInfo;

  SystemInfoTest()
      : otaEnabled(false),
        preferencesStorage(),  // Uses default test values
        systemInfo(preferencesStorage, otaEnabled) {}

  // No SetUp/TearDown needed - constructor initialization is sufficient
};

// ============================================================================
// getSpecialCharacterSettings Tests
// ============================================================================

TEST_F(SystemInfoTest, GetSpecialCharacterSettingsReturnsCorrectFormat) {
  const auto settings = systemInfo.getSpecialCharacterSettings();

  // Verify all command descriptions are present
  EXPECT_NE(settings.find("Ctrl+Y i:"), types::string::npos);
  EXPECT_NE(settings.find("Ctrl+Y d:"), types::string::npos);
  EXPECT_NE(settings.find("Ctrl+Y b:"), types::string::npos);
  EXPECT_NE(settings.find("Ctrl+Y n"), types::string::npos);  // Note: no colon
}

// ============================================================================
// getWelcomeString Tests
// ============================================================================

TEST_F(SystemInfoTest, GetWelcomeStringGeneratesCompleteFormat) {
  const auto welcome = systemInfo.getWelcomeString();

  // Verify main sections are present
  EXPECT_NE(welcome.find("========================================"), types::string::npos);
  EXPECT_NE(welcome.find("ESP32-C3 Serial Bridge"), types::string::npos);
  EXPECT_NE(welcome.find("Serial:"), types::string::npos);
  EXPECT_NE(welcome.find("Settings JSON:"), types::string::npos);
  EXPECT_NE(welcome.find("OTA:"), types::string::npos);

  // Verify special characters section is included
  EXPECT_NE(welcome.find("Special characters for USB interface"), types::string::npos);
  EXPECT_NE(welcome.find("Ctrl+Y i:"), types::string::npos);
  EXPECT_NE(welcome.find("Ctrl+Y d:"), types::string::npos);
  EXPECT_NE(welcome.find("Ctrl+Y b:"), types::string::npos);
  EXPECT_NE(welcome.find("Ctrl+Y n"), types::string::npos);

  // Verify reset command description
  EXPECT_NE(welcome.find("reset the device"), types::string::npos);
  EXPECT_NE(welcome.find("operation can be cancelled"), types::string::npos);
}

// ============================================================================
// logSystemInformation Tests
// ============================================================================

TEST_F(SystemInfoTest, LogSystemInformationDoesNotCrash) {
  // Verify the method completes without throwing
  // Note: Cannot verify log output without mocking the logger infrastructure
  EXPECT_NO_THROW(systemInfo.logSystemInformation());
}

} // namespace
} // namespace jrb::wifi_serial
