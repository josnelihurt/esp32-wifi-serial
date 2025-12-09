#pragma once

#include "interfaces/itask.h"
#include "interfaces/imqtt_client.h"
#include "domain/config/preferences_storage.h"

namespace jrb::wifi_serial {

class MqttReconnectTask final : public ITask {
private:
    IMqttClient* mqttClient;
    PreferencesStorage& preferencesStorage;

public:
    MqttReconnectTask(IMqttClient* client, PreferencesStorage& storage)
        : mqttClient(client), preferencesStorage(storage) {}
    
    void loop() override;
};

}  // namespace jrb::wifi_serial

