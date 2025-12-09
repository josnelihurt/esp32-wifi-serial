#pragma once

#include "config.h"
#include "preferences_storage.h"
#include "mqtt_client.h"
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

class SerialCommandHandler final {
private:
    PreferencesStorage& configManager;
    MqttClient* mqttHandler;
    bool& debugEnabled;
    bool cmdPrefixReceived{false};
    std::function<void()> printWelcomeFunc;

public:
    SerialCommandHandler(PreferencesStorage& config, MqttClient* handler, 
                        bool& debug, std::function<void()> printWelcome)
        : configManager(config), mqttHandler(handler), debugEnabled(debug), 
          printWelcomeFunc(printWelcome) {}
    
    void handle();
};

}  // namespace jrb::wifi_serial
