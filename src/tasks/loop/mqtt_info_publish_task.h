#pragma once

#include "interfaces/itask.h"
#include "interfaces/imqtt_client.h"
#include "domain/config/preferences_storage.h"
#include <WiFi.h>
#include <Arduino.h>
#include <ArduinoLog.h>
namespace jrb::wifi_serial {

class IWiFiManager;

class MqttInfoPublishTask final : public ITask {
private:
    IMqttClient& mqttClient;
    IWiFiManager& wifiManager;
    PreferencesStorage& preferencesStorage;
    unsigned long& lastInfoPublish;
    static constexpr unsigned long INFO_PUBLISH_INTERVAL_MS = 30000;

public:
    MqttInfoPublishTask(IMqttClient& client, IWiFiManager& wifi, PreferencesStorage& storage, unsigned long& lastPublish);
    ~MqttInfoPublishTask() = default;
    void setup() override;
    void loop() override;
};

}  // namespace jrb::wifi_serial

