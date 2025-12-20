#include "preferences_storage.h"
#include "config.h"
#include "infrastructure/logging/logger.h"
#include "preferences_storage_policy.h"

namespace jrb::wifi_serial {

// ============================================================================
// Constructor
// ============================================================================

template <typename StoragePolicy>
PreferencesStorage<StoragePolicy>::PreferencesStorage()
    : deviceName{DEFAULT_DEVICE_NAME}, baudRateTty1{DEFAULT_BAUD_RATE_TTY1},
      mqttBroker{}, mqttPort{DEFAULT_MQTT_PORT}, mqttUser{}, mqttPassword{},
      topicTty0Rx{}, topicTty0Tx{}, topicTty1Rx{}, topicTty1Tx{}, ssid{},
      password{}, webUser{"admin"}, webPassword{}, debugEnabled{false},
      tty02tty1Bridge{false} {
  load();
}

// ============================================================================
// Private Methods
// ============================================================================

template <typename StoragePolicy>
void PreferencesStorage<StoragePolicy>::load() {
  LOG_INFO("Loading preferences");

  storage.begin("esp32bridge", true);

  deviceName = storage.getString("deviceName", DEFAULT_DEVICE_NAME);
  baudRateTty1 = storage.getInt("baudRateTty1", DEFAULT_BAUD_RATE_TTY1);
  mqttBroker = storage.getString("mqttBroker", "");
  mqttPort = storage.getInt("mqttPort", DEFAULT_MQTT_PORT);
  mqttUser = storage.getString("mqttUser", "");
  mqttPassword = storage.getString("mqttPassword", "");

  topicTty0Rx = storage.getString("topicTty0Rx", "");
  topicTty0Tx = storage.getString("topicTty0Tx", "");
  topicTty1Rx = storage.getString("topicTty1Rx", "");
  topicTty1Tx = storage.getString("topicTty1Tx", "");
  ssid = storage.getString("ssid", "");
  password = storage.getString("password", "");
  webUser = storage.getString("webUser", "admin");
  webPassword = storage.getString("webPassword", "");

  storage.end();
  generateDefaultTopics();
}

template <typename StoragePolicy>
void PreferencesStorage<StoragePolicy>::generateDefaultTopics() {
  LOG_INFO("Generating default topics with device name: %s",
           deviceName.c_str());

  char baseTopic0[64], baseTopic1[64];
  snprintf(baseTopic0, sizeof(baseTopic0), DEFAULT_TOPIC_TTY0,
           deviceName.c_str());
  snprintf(baseTopic1, sizeof(baseTopic1), DEFAULT_TOPIC_TTY1,
           deviceName.c_str());

  std::string base0 = std::string(baseTopic0);
  std::string base1 = std::string(baseTopic1);

  // Ensure topics start with "wifi_serial/"
  if (base0.find("wifi_serial/") != 0) {
    base0 = "wifi_serial/" + base0;
  }
  if (base1.find("wifi_serial/") != 0) {
    base1 = "wifi_serial/" + base1;
  }

  if (topicTty0Rx.empty()) {
    topicTty0Rx = base0 + "/rx";
  } else if (topicTty0Rx.find("wifi_serial/") != 0) {
    topicTty0Rx = "wifi_serial/" + topicTty0Rx;
  }

  if (topicTty0Tx.empty()) {
    topicTty0Tx = base0 + "/tx";
  } else if (topicTty0Tx.find("wifi_serial/") != 0) {
    topicTty0Tx = "wifi_serial/" + topicTty0Tx;
  }

  if (topicTty1Rx.empty()) {
    topicTty1Rx = base1 + "/rx";
  } else if (topicTty1Rx.find("wifi_serial/") != 0) {
    topicTty1Rx = "wifi_serial/" + topicTty1Rx;
  }

  if (topicTty1Tx.empty()) {
    topicTty1Tx = base1 + "/tx";
  } else if (topicTty1Tx.find("wifi_serial/") != 0) {
    topicTty1Tx = "wifi_serial/" + topicTty1Tx;
  }
}

// ============================================================================
// Public Methods
// ============================================================================

template <typename StoragePolicy>
std::string PreferencesStorage<StoragePolicy>::serialize(
    const std::string &ipAddress, const std::string &macAddress,
    const std::string &ssid) const {

  // Delegate to the policy's JSON serialization implementation
  return storage.serializeJson(deviceName, mqttBroker, mqttPort, mqttUser,
                               mqttPassword, topicTty0Rx, topicTty0Tx,
                               topicTty1Rx, topicTty1Tx, ipAddress, macAddress,
                               ssid, password, webUser, webPassword,
                               debugEnabled, tty02tty1Bridge);
}

template <typename StoragePolicy>
void PreferencesStorage<StoragePolicy>::save() {
  storage.begin("esp32bridge", false);

  storage.putString("deviceName", deviceName);
  storage.putInt("baudRateTty1", baudRateTty1);
  storage.putString("mqttBroker", mqttBroker);
  storage.putInt("mqttPort", mqttPort);
  storage.putString("mqttUser", mqttUser);
  storage.putString("mqttPassword", mqttPassword);
  storage.putString("topicTty0Rx", topicTty0Rx);
  storage.putString("topicTty0Tx", topicTty0Tx);
  storage.putString("topicTty1Rx", topicTty1Rx);
  storage.putString("topicTty1Tx", topicTty1Tx);
  storage.putString("ssid", ssid);
  storage.putString("password", password);
  storage.putString("webUser", webUser);
  storage.putString("webPassword", webPassword);

  storage.end();
}

template <typename StoragePolicy>
void PreferencesStorage<StoragePolicy>::clear() {
  storage.begin("esp32bridge", false);
  storage.clear();
  storage.end();

  deviceName = DEFAULT_DEVICE_NAME;
  baudRateTty1 = DEFAULT_BAUD_RATE_TTY1;
  mqttBroker = "";
  mqttPort = DEFAULT_MQTT_PORT;
  mqttUser = "";
  mqttPassword = "";
  topicTty0Rx = "";
  topicTty0Tx = "";
  topicTty1Rx = "";
  topicTty1Tx = "";
  ssid = "";
  password = "";
  webUser = "admin";
  webPassword = "";
  debugEnabled = false;
  tty02tty1Bridge = false;
}

} // namespace jrb::wifi_serial
