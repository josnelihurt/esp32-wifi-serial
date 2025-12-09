#pragma once

#include "interfaces/itask.h"
#include "mqtt_client.h"
#include "preferences_storage.h"

namespace jrb::wifi_serial {

class MqttReconnectTask final : public ITask {
private:
    MqttClient* mqttClient;
    PreferencesStorage& preferencesStorage;

public:
    MqttReconnectTask(MqttClient* client, PreferencesStorage& storage)
        : mqttClient(client), preferencesStorage(storage) {}
    
    void loop() override;
};

}  // namespace jrb::wifi_serial

