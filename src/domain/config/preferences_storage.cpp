#include "preferences_storage.h"
#include "config.h"

// Include appropriate policy based on platform
#ifdef ESP_PLATFORM
#include "preferences_storage_policy_esp32.h"
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#else
#include "preferences_storage_policy_test.h"
#include <sstream>
#endif

namespace jrb::wifi_serial {

// Define static storage for TestStoragePolicy
#ifndef ESP_PLATFORM
std::map<std::string, std::variant<std::string, int32_t>>
    TestStoragePolicy::storage;
#endif

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
#ifdef ESP_PLATFORM
  Log.infoln("Loading preferences");
#endif

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
#ifdef ESP_PLATFORM
  Log.infoln("Generating default topics with device name: %s",
             deviceName.c_str());
#endif

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

#ifdef ESP_PLATFORM
  // Use ArduinoJson for ESP32
  String output;
  StaticJsonDocument<1024> obj;
  obj["deviceName"] = deviceName.c_str();
  obj["mqttBroker"] = mqttBroker.c_str();
  obj["mqttPort"] = mqttPort;
  obj["mqttUser"] = mqttUser.c_str();
  obj["mqttPassword"] =
      mqttPassword.length() > 0 ? "********" : "NO_PASSWORD";
  obj["topicTty0Rx"] = topicTty0Rx.c_str();
  obj["topicTty0Tx"] = topicTty0Tx.c_str();
  obj["topicTty1Rx"] = topicTty1Rx.c_str();
  obj["topicTty1Tx"] = topicTty1Tx.c_str();
  obj["ipAddress"] = ipAddress.c_str();
  obj["macAddress"] = macAddress.c_str();
  obj["ssid"] = ssid.c_str();
  obj["mqtt"] = (mqttBroker.length() > 0 ? "connected" : "disconnected");
  obj["password"] = password.length() > 0 ? "********" : "NO_PASSWORD";
  obj["webUser"] = webUser.c_str();
  obj["webPassword"] = webPassword.length() > 0 ? "********" : "NO_PASSWORD";
  obj["debugEnabled"] = debugEnabled;
  obj["tty02tty1Bridge"] = tty02tty1Bridge;
  serializeJsonPretty(obj, output);
  return std::string(output.c_str());
#else
  // Simple JSON serialization for testing (no ArduinoJson dependency)
  std::ostringstream oss;
  oss << "{\n"
      << "  \"deviceName\": \"" << deviceName << "\",\n"
      << "  \"mqttBroker\": \"" << mqttBroker << "\",\n"
      << "  \"mqttPort\": " << mqttPort << ",\n"
      << "  \"mqttUser\": \"" << mqttUser << "\",\n"
      << "  \"mqttPassword\": \""
      << (mqttPassword.empty() ? "NO_PASSWORD" : "********") << "\",\n"
      << "  \"topicTty0Rx\": \"" << topicTty0Rx << "\",\n"
      << "  \"topicTty0Tx\": \"" << topicTty0Tx << "\",\n"
      << "  \"topicTty1Rx\": \"" << topicTty1Rx << "\",\n"
      << "  \"topicTty1Tx\": \"" << topicTty1Tx << "\",\n"
      << "  \"ipAddress\": \"" << ipAddress << "\",\n"
      << "  \"macAddress\": \"" << macAddress << "\",\n"
      << "  \"ssid\": \"" << ssid << "\",\n"
      << "  \"mqtt\": \"" << (mqttBroker.empty() ? "disconnected" : "connected")
      << "\",\n"
      << "  \"password\": \"" << (password.empty() ? "NO_PASSWORD" : "********")
      << "\",\n"
      << "  \"webUser\": \"" << webUser << "\",\n"
      << "  \"webPassword\": \""
      << (webPassword.empty() ? "NO_PASSWORD" : "********") << "\",\n"
      << "  \"debugEnabled\": " << (debugEnabled ? "true" : "false") << ",\n"
      << "  \"tty02tty1Bridge\": " << (tty02tty1Bridge ? "true" : "false")
      << "\n"
      << "}";
  return oss.str();
#endif
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

// ============================================================================
// Explicit Template Instantiation
// ============================================================================

#ifdef ESP_PLATFORM
// Instantiate for ESP32 platform
template class PreferencesStorage<ESP32StoragePolicy>;
#else
// Instantiate for test platform
template class PreferencesStorage<TestStoragePolicy>;
#endif

} // namespace jrb::wifi_serial
