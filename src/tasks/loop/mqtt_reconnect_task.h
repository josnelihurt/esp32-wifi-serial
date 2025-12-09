#pragma once

#include "interfaces/itask.h"
#include "mqtt_client.h"
#include "preferences_storage.h"

namespace jrb::wifi_serial {

class MqttReconnectTask final : public ITask {
private:
    MqttClient* mqttHandler;
    PreferencesStorage& configManager;

public:
    MqttReconnectTask(MqttClient* handler, PreferencesStorage& config)
        : mqttHandler(handler), configManager(config) {}
    
    void loop() override;
};

}  // namespace jrb::wifi_serial

