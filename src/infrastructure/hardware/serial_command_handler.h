#pragma once

#include "config.h"
#include "domain/config/preferences_storage.h"
#include "interfaces/imqtt_client.h"
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

class SerialCommandHandler final {
private:
    PreferencesStorage& preferencesStorage;
    IMqttClient* mqttClient;
    bool& debugEnabled;
    bool cmdPrefixReceived{false};
    std::function<void()> printWelcomeFunc;

public:
    SerialCommandHandler(PreferencesStorage& storage, IMqttClient* client,
                        bool& debug, std::function<void()> printWelcome)
        : preferencesStorage(storage), mqttClient(client), debugEnabled(debug),
          printWelcomeFunc(printWelcome) {}

    void handle();
};

}  // namespace jrb::wifi_serial
