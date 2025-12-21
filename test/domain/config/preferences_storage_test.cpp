#include "domain/config/preferences_storage.cpp" // Include implementation for tests
#include <gtest/gtest.h>

namespace jrb::wifi_serial {
namespace {

class PreferencesStorageTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Each test gets a fresh instance
  }

  void TearDown() override {
    // Cleanup if needed
  }
};

// ============================================================================
// Constructor and Initial State Tests
// ============================================================================

TEST_F(PreferencesStorageTest, ConstructorLoadsDefaults) {
  PreferencesStorageDefault storage;

  EXPECT_EQ(storage.deviceName, DEFAULT_DEVICE_NAME);
  EXPECT_EQ(storage.baudRateTty1, DEFAULT_BAUD_RATE_TTY1);
  EXPECT_EQ(storage.mqttPort, DEFAULT_MQTT_PORT);
  EXPECT_EQ(storage.webUser, "admin");
  EXPECT_FALSE(storage.debugEnabled);
  EXPECT_FALSE(storage.tty02tty1Bridge);
}

TEST_F(PreferencesStorageTest, ConstructorGeneratesDefaultTopics) {
  PreferencesStorageDefault storage;

  // Topics should be generated based on device name
  EXPECT_FALSE(storage.topicTty0Rx.empty());
  EXPECT_FALSE(storage.topicTty0Tx.empty());
  EXPECT_FALSE(storage.topicTty1Rx.empty());
  EXPECT_FALSE(storage.topicTty1Tx.empty());

  // Topics should start with "wifi_serial/"
  EXPECT_EQ(storage.topicTty0Rx.find("wifi_serial/"), 0);
  EXPECT_EQ(storage.topicTty0Tx.find("wifi_serial/"), 0);
  EXPECT_EQ(storage.topicTty1Rx.find("wifi_serial/"), 0);
  EXPECT_EQ(storage.topicTty1Tx.find("wifi_serial/"), 0);

  // Topics should end with /rx or /tx
  EXPECT_NE(storage.topicTty0Rx.find("/rx"), types::string::npos);
  EXPECT_NE(storage.topicTty0Tx.find("/tx"), types::string::npos);
  EXPECT_NE(storage.topicTty1Rx.find("/rx"), types::string::npos);
  EXPECT_NE(storage.topicTty1Tx.find("/tx"), types::string::npos);
}

// ============================================================================
// Save and Load Tests
// ============================================================================

TEST_F(PreferencesStorageTest, SaveStoresAllFields) {
  PreferencesStorageDefault storage;

  // Modify all fields
  storage.deviceName = "test-device";
  storage.baudRateTty1 = 115200;
  storage.mqttBroker = "test.mqtt.broker";
  storage.mqttPort = 1883;
  storage.mqttUser = "testuser";
  storage.mqttPassword = "testpass";
  storage.topicTty0Rx = "wifi_serial/test/tty0/rx";
  storage.topicTty0Tx = "wifi_serial/test/tty0/tx";
  storage.topicTty1Rx = "wifi_serial/test/tty1/rx";
  storage.topicTty1Tx = "wifi_serial/test/tty1/tx";
  storage.ssid = "TestWiFi";
  storage.password = "wifipass";
  storage.webUser = "webadmin";
  storage.webPassword = "webpass";
  storage.debugEnabled = true;
  storage.tty02tty1Bridge = true;

  // Save should not throw
  EXPECT_NO_THROW(storage.save());

  // Create new instance - should load saved values
  PreferencesStorageDefault storage2;

  EXPECT_EQ(storage2.deviceName, "test-device");
  EXPECT_EQ(storage2.baudRateTty1, 115200);
  EXPECT_EQ(storage2.mqttBroker, "test.mqtt.broker");
  EXPECT_EQ(storage2.mqttPort, 1883);
  EXPECT_EQ(storage2.mqttUser, "testuser");
  EXPECT_EQ(storage2.mqttPassword, "testpass");
  EXPECT_EQ(storage2.ssid, "TestWiFi");
  EXPECT_EQ(storage2.password, "wifipass");
  EXPECT_EQ(storage2.webUser, "webadmin");
  EXPECT_EQ(storage2.webPassword, "webpass");
}

// ============================================================================
// Clear Tests
// ============================================================================

TEST_F(PreferencesStorageTest, ClearResetsToDefaults) {
  PreferencesStorageDefault storage;

  // Modify fields
  storage.deviceName = "modified";
  storage.mqttBroker = "broker";
  storage.debugEnabled = true;
  storage.save();

  // Clear
  storage.clear();

  // Should be reset to defaults
  EXPECT_EQ(storage.deviceName, DEFAULT_DEVICE_NAME);
  EXPECT_EQ(storage.baudRateTty1, DEFAULT_BAUD_RATE_TTY1);
  EXPECT_EQ(storage.mqttBroker, "");
  EXPECT_EQ(storage.mqttPort, DEFAULT_MQTT_PORT);
  EXPECT_EQ(storage.webUser, "admin");
  EXPECT_FALSE(storage.debugEnabled);
  EXPECT_FALSE(storage.tty02tty1Bridge);
}

// ============================================================================
// Serialize Tests
// ============================================================================

TEST_F(PreferencesStorageTest, SerializeCreatesValidJSON) {
  PreferencesStorageDefault storage;

  storage.deviceName = "test-device";
  storage.mqttBroker = "mqtt.example.com";

  types::string json = storage.serialize("192.168.1.100", "AA:BB:CC:DD:EE:FF",
                                       "TestNetwork");

  // Should contain JSON structure
  EXPECT_NE(json.find("{"), types::string::npos);
  EXPECT_NE(json.find("}"), types::string::npos);

  // Should contain key fields
  EXPECT_NE(json.find("deviceName"), types::string::npos);
  EXPECT_NE(json.find("test-device"), types::string::npos);
  EXPECT_NE(json.find("mqttBroker"), types::string::npos);
  EXPECT_NE(json.find("ipAddress"), types::string::npos);
  EXPECT_NE(json.find("192.168.1.100"), types::string::npos);
  EXPECT_NE(json.find("macAddress"), types::string::npos);
  EXPECT_NE(json.find("AA:BB:CC:DD:EE:FF"), types::string::npos);
}

TEST_F(PreferencesStorageTest, SerializeMasksPasswords) {
  PreferencesStorageDefault storage;

  storage.mqttPassword = "secret123";
  storage.password = "wifisecret";
  storage.webPassword = "websecret";

  types::string json = storage.serialize("", "", "");

  // Passwords should be masked
  EXPECT_EQ(json.find("secret123"), types::string::npos);
  EXPECT_EQ(json.find("wifisecret"), types::string::npos);
  EXPECT_EQ(json.find("websecret"), types::string::npos);

  // Should contain masked indicators
  EXPECT_NE(json.find("********"), types::string::npos);
}

// ============================================================================
// Topic Generation Tests
// ============================================================================

TEST_F(PreferencesStorageTest, GeneratesTopicsWithWifiSerialPrefix) {
  PreferencesStorageDefault storage;

  storage.deviceName = "custom-device";
  // Force regeneration by creating new instance
  PreferencesStorageDefault storage2;
  storage2.deviceName = "custom-device";
  storage2.topicTty0Rx = "";
  storage2.topicTty0Tx = "";

  // Manually trigger topic generation (in real code, this happens in load())
  // We can test by checking the prefix logic works

  EXPECT_EQ(storage.topicTty0Rx.substr(0, 12), "wifi_serial/");
  EXPECT_EQ(storage.topicTty0Tx.substr(0, 12), "wifi_serial/");
  EXPECT_EQ(storage.topicTty1Rx.substr(0, 12), "wifi_serial/");
  EXPECT_EQ(storage.topicTty1Tx.substr(0, 12), "wifi_serial/");
}

TEST_F(PreferencesStorageTest, PreservesCustomTopicsWithPrefix) {
  PreferencesStorageDefault storage;

  storage.topicTty0Rx = "wifi_serial/custom/rx";
  storage.save();

  PreferencesStorageDefault storage2;
  EXPECT_EQ(storage2.topicTty0Rx, "wifi_serial/custom/rx");
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(PreferencesStorageTest, HandlesEmptyStrings) {
  PreferencesStorageDefault storage;

  storage.mqttBroker = "";
  storage.mqttUser = "";
  storage.mqttPassword = "";

  EXPECT_NO_THROW(storage.save());
  EXPECT_NO_THROW(storage.serialize("", "", ""));
}

TEST_F(PreferencesStorageTest, HandlesSpecialCharacters) {
  PreferencesStorageDefault storage;

  storage.deviceName = "device-with_special.chars";
  storage.mqttUser = "user@domain.com";

  EXPECT_NO_THROW(storage.save());

  PreferencesStorageDefault storage2;
  EXPECT_EQ(storage2.deviceName, "device-with_special.chars");
  EXPECT_EQ(storage2.mqttUser, "user@domain.com");
}

} // namespace
} // namespace jrb::wifi_serial
