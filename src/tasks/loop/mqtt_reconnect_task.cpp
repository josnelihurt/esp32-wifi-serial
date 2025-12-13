#include "mqtt_reconnect_task.h"
#include "domain/network/wifi_manager.h"
#include <ArduinoLog.h>
namespace jrb::wifi_serial {

MqttReconnectTask::MqttReconnectTask(IMqttClient &client, IWiFiManager &wifi,
                                     PreferencesStorage &storage)
    : mqttClient(client), wifiManager(wifi), preferencesStorage(storage) {
        Log.traceln(__PRETTY_FUNCTION__);
    }

void MqttReconnectTask::loop() {
  if (wifiManager.isAPMode())
    return;
  if (preferencesStorage.mqttBroker.length() == 0)
    return;
  if (mqttClient.isConnected())
    return;

  static unsigned long lastReconnectAttempt = 0;
  unsigned long now = millis();

  if (now - lastReconnectAttempt < 5000) {
    return;
  }
  lastReconnectAttempt = now;

  Log.warningln("%s: Attempting to reconnect...", __PRETTY_FUNCTION__);
  const char *user = preferencesStorage.mqttUser.length() > 0
                         ? preferencesStorage.mqttUser.c_str()
                         : nullptr;
  const char *pass = preferencesStorage.mqttPassword.length() > 0
                         ? preferencesStorage.mqttPassword.c_str()
                         : nullptr;

  bool result = mqttClient.connect(preferencesStorage.mqttBroker.c_str(),
                                   preferencesStorage.mqttPort, user, pass);
  if (!result) {
    Log.errorln("%s: Reconnect failed", __PRETTY_FUNCTION__);
    return;
  }
  Log.infoln("%s: Reconnected successfully!", __PRETTY_FUNCTION__);
}

} // namespace jrb::wifi_serial
