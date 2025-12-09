#pragma once

#include "interfaces/itask.h"
#include "mqtt_client.h"
#include "preferences_storage.h"
#include <WiFi.h>
#include <Arduino.h>

namespace jrb::wifi_serial {

class MqttInfoPublishTask final : public ITask {
private:
    MqttClient* mqttClient;
    PreferencesStorage& preferencesStorage;
    unsigned long& lastInfoPublish;
    static constexpr unsigned long INFO_PUBLISH_INTERVAL = 30000;

public:
    MqttInfoPublishTask(MqttClient* client, PreferencesStorage& storage, unsigned long& lastPublish)
        : mqttClient(client), preferencesStorage(storage), lastInfoPublish(lastPublish) {}
    
    void loop() override;
};

}  // namespace jrb::wifi_serial

