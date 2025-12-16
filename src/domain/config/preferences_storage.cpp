#include "preferences_storage.h"
#include "config.h"
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <algorithm>
namespace jrb::wifi_serial {

PreferencesStorage::PreferencesStorage()
    : deviceName{DEFAULT_DEVICE_NAME}, mqttBroker{},
      mqttPort{DEFAULT_MQTT_PORT}, mqttUser{}, mqttPassword{}, topicTty0Rx{},
      topicTty0Tx{}, topicTty1Rx{}, topicTty1Tx{}, webUser{"admin"},
      webPassword{}, debugEnabled{false}, tty02tty1Bridge{false} {
  load();
}

void PreferencesStorage::load() {
  Log.infoln("Loading preferences");
  preferences.begin("esp32bridge", true);

  deviceName = preferences.getString("deviceName", DEFAULT_DEVICE_NAME);
  baudRateTty1 = preferences.getInt("baudRateTty1", DEFAULT_BAUD_RATE_TTY1);
  mqttBroker = preferences.getString("mqttBroker", "");
  mqttPort = preferences.getInt("mqttPort", DEFAULT_MQTT_PORT);
  mqttUser = preferences.getString("mqttUser", "");
  mqttPassword = preferences.getString("mqttPassword", "");

  topicTty0Rx = preferences.getString("topicTty0Rx", "");
  topicTty0Tx = preferences.getString("topicTty0Tx", "");
  topicTty1Rx = preferences.getString("topicTty1Rx", "");
  topicTty1Tx = preferences.getString("topicTty1Tx", "");
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  webUser = preferences.getString("webUser", "admin");
  webPassword = preferences.getString("webPassword", "");

  preferences.end();
  generateDefaultTopics();
}

String PreferencesStorage::serialize(const String &ipAddress,
                                     const String &macAddress,
                                     const String &ssid) {
  String output;
  StaticJsonDocument<1024> obj;
  obj["deviceName"] = deviceName;
  obj["mqttBroker"] = mqttBroker;
  obj["mqttPort"] = mqttPort;
  obj["mqttUser"] = mqttUser;
  obj["mqttPassword"] = mqttPassword.length() > 0 ? "NO_PASSWORD" : "********";
  obj["topicTty0Rx"] = topicTty0Rx;
  obj["topicTty0Tx"] = topicTty0Tx;
  obj["topicTty1Rx"] = topicTty1Rx;
  obj["topicTty1Tx"] = topicTty1Tx;
  obj["ipAddress"] = ipAddress;
  obj["macAddress"] = macAddress;
  obj["ssid"] = ssid;
  obj["mqtt"] = (mqttBroker.length() > 0 ? "connected" : "disconnected");
  obj["password"] = password.length() > 0 ? "NO_PASSWORD" : "********";
  obj["webUser"] = webUser;
  obj["webPassword"] = webPassword.length() > 0 ? "NO_PASSWORD" : "** ******";
  obj["debugEnabled"] = debugEnabled;
  obj["tty02tty1Bridge"] = tty02tty1Bridge;
  serializeJsonPretty(obj, output);
  return output;
}

void PreferencesStorage::generateDefaultTopics() {
  Log.infoln("Generating default topics with device name: %s",
             deviceName.c_str());
  char baseTopic0[64], baseTopic1[64];
  snprintf(baseTopic0, sizeof(baseTopic0), DEFAULT_TOPIC_TTY0,
           deviceName.c_str());
  snprintf(baseTopic1, sizeof(baseTopic1), DEFAULT_TOPIC_TTY1,
           deviceName.c_str());

  String base0 = String(baseTopic0);
  String base1 = String(baseTopic1);

  if (!base0.startsWith("wifi_serial/")) {
    base0 = "wifi_serial/" + base0;
  }
  if (!base1.startsWith("wifi_serial/")) {
    base1 = "wifi_serial/" + base1;
  }

  if (topicTty0Rx.length() == 0) {
    topicTty0Rx = base0 + "/rx";
  } else if (!topicTty0Rx.startsWith("wifi_serial/")) {
    topicTty0Rx = "wifi_serial/" + topicTty0Rx;
  }

  if (topicTty0Tx.length() == 0) {
    topicTty0Tx = base0 + "/tx";
  } else if (!topicTty0Tx.startsWith("wifi_serial/")) {
    topicTty0Tx = "wifi_serial/" + topicTty0Tx;
  }

  if (topicTty1Rx.length() == 0) {
    topicTty1Rx = base1 + "/rx";
  } else if (!topicTty1Rx.startsWith("wifi_serial/")) {
    topicTty1Rx = "wifi_serial/" + topicTty1Rx;
  }

  if (topicTty1Tx.length() == 0) {
    topicTty1Tx = base1 + "/tx";
  } else if (!topicTty1Tx.startsWith("wifi_serial/")) {
    topicTty1Tx = "wifi_serial/" + topicTty1Tx;
  }
}

void PreferencesStorage::save() {
  preferences.begin("esp32bridge", false);

  preferences.putString("deviceName", deviceName);
  preferences.putInt("baudRateTty1", baudRateTty1);
  preferences.putString("mqttBroker", mqttBroker);
  preferences.putInt("mqttPort", mqttPort);
  preferences.putString("mqttUser", mqttUser);
  preferences.putString("mqttPassword", mqttPassword);
  preferences.putString("topicTty0Rx", topicTty0Rx);
  preferences.putString("topicTty0Tx", topicTty0Tx);
  preferences.putString("topicTty1Rx", topicTty1Rx);
  preferences.putString("topicTty1Tx", topicTty1Tx);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.putString("webUser", webUser);
  preferences.putString("webPassword", webPassword);
  preferences.end();
}

void PreferencesStorage::clear() {
  preferences.begin("esp32bridge", false);
  preferences.clear();
  preferences.end();

  deviceName = DEFAULT_DEVICE_NAME;
  mqttBroker = "";
  mqttPort = DEFAULT_MQTT_PORT;
  mqttUser = "";
  mqttPassword = "";
  topicTty0Rx = "";
  topicTty0Tx = "";
  topicTty1Rx = "";
  topicTty1Tx = "";
  webUser = "admin";
  webPassword = "";
}

} // namespace jrb::wifi_serial
