#pragma once

#include "domain/config/preferences_storage.h"
#include "domain/network/mqtt_client.h"
#include <Arduino.h>
#include <WiFi.h>

namespace jrb::wifi_serial {

class SerialBufferManager;

class SystemInfo final {
private:
  PreferencesStorage &preferencesStorage;
  MqttClient &mqttClient;
  bool &otaEnabled;

public:
  SystemInfo(PreferencesStorage &storage, MqttClient &client, bool &ota)
      : preferencesStorage(storage), mqttClient(client), otaEnabled(ota) {}

  void logSystemInformation() const;
  String getSpecialCharacterSettings() const;
  String getWelcomeString() const;
};

} // namespace jrb::wifi_serial
