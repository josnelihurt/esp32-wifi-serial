#pragma once

#include "interfaces/itask.h"
#include "interfaces/imqtt_client.h"
#include "domain/config/preferences_storage.h"

namespace jrb::wifi_serial {
class IWiFiManager;
class MqttReconnectTask final : public ITask {
private:
    IMqttClient& mqttClient;
    IWiFiManager& wifiManager;
    PreferencesStorage& preferencesStorage;

public: 
    MqttReconnectTask(IMqttClient& client, IWiFiManager& wifi, PreferencesStorage& storage);
    ~MqttReconnectTask() = default;
    
    void loop() override;
};

}  // namespace jrb::wifi_serial

