#include "mqtt_info_publish_task.h"
#include "domain/network/wifi_manager.h"
#include <ArduinoLog.h>

namespace jrb::wifi_serial {
MqttInfoPublishTask::MqttInfoPublishTask(IMqttClient &client,
                                         IWiFiManager &wifi,
                                         PreferencesStorage &storage,
                                         unsigned long &lastPublish)
    : mqttClient(client), wifiManager(wifi), preferencesStorage(storage),
      lastInfoPublish(lastPublish) {
  Log.traceln(__PRETTY_FUNCTION__);
}

void MqttInfoPublishTask::setup() {
  Log.traceln(__PRETTY_FUNCTION__);
  lastInfoPublish = millis();
}

void MqttInfoPublishTask::loop() {
  if (wifiManager.isAPMode()) return;
  if (!mqttClient.isConnected()) {
    Log.errorln("MQTT client not connected");
    delay(1000);
    return;
  }
  if (lastInfoPublish + INFO_PUBLISH_INTERVAL_MS >= millis())
    return;

  Log.verboseln("%s: Publishing info", __PRETTY_FUNCTION__);
  String macAddress = WiFi.macAddress();
  String ipAddress = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString()
                                                   : "Not connected";
  String ssid = WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "Not configured";

  String infoJson =
      "{\"device\":\"" + preferencesStorage.deviceName + "\",\"ip\":\"" +
      ipAddress + "\",\"mac\":\"" + macAddress + "\",\"ssid\":\"" + ssid +
      "\",\"mqtt\":\"" +
      (preferencesStorage.mqttBroker.length() > 0 ? "connected"
                                                    : "disconnected") +
      "\"}";
  mqttClient.publishInfo(infoJson);
  lastInfoPublish = millis();
}

} // namespace jrb::wifi_serial
