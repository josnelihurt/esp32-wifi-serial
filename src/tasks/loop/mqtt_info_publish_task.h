#pragma once

#include "interfaces/itask.h"
#include "mqtt_client.h"
#include "preferences_storage.h"
#include <WiFi.h>
#include <Arduino.h>

namespace jrb::wifi_serial {

class MqttInfoPublishTask final : public ITask {
private:
    MqttClient* mqttHandler;
    PreferencesStorage& configManager;
    unsigned long& lastInfoPublish;
    static constexpr unsigned long INFO_PUBLISH_INTERVAL = 30000;

public:
    MqttInfoPublishTask(MqttClient* handler, PreferencesStorage& config, unsigned long& lastPublish)
        : mqttHandler(handler), configManager(config), lastInfoPublish(lastPublish) {}
    
    void loop() override;
};

}  // namespace jrb::wifi_serial

