#pragma once

#include "config.h"
#include "domain/config/preferences_storage.h"
#include "domain/network/mqtt_client.h"
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

class SerialCommandHandler final {
private:
    PreferencesStorage& preferencesStorage;
    MqttClient& mqttClient;
    bool& debugEnabled;
    bool cmdPrefixReceived{false};
    std::function<void()> printWelcomeFunc;

public:
    SerialCommandHandler(PreferencesStorage& storage, MqttClient& client,
                        bool& debug, std::function<void()> printWelcome)
        : preferencesStorage(storage), mqttClient(client), debugEnabled(debug),
          printWelcomeFunc(printWelcome) {}

    void handle();
};

}  // namespace jrb::wifi_serial
