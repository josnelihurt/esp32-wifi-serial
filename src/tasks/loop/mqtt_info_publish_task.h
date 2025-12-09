#pragma once

#include "interfaces/itask.h"
#include "interfaces/imqtt_client.h"
#include "domain/config/preferences_storage.h"
#include <WiFi.h>
#include <Arduino.h>

namespace jrb::wifi_serial {

class MqttInfoPublishTask final : public ITask {
private:
    IMqttClient* mqttClient;
    PreferencesStorage& preferencesStorage;
    unsigned long& lastInfoPublish;
    static constexpr unsigned long INFO_PUBLISH_INTERVAL = 30000;

public:
    MqttInfoPublishTask(IMqttClient* client, PreferencesStorage& storage, unsigned long& lastPublish)
        : mqttClient(client), preferencesStorage(storage), lastInfoPublish(lastPublish) {}
    
    void loop() override;
};

}  // namespace jrb::wifi_serial

