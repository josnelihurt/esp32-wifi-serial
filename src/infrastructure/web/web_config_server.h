#pragma once

#include "domain/config/preferences_storage.h"
#include "domain/serial/serial_log.h"
#include <ESPAsyncWebServer.h>
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

class WebConfigServer final {
public:
    WebConfigServer(PreferencesStorage& storage, SerialLog& serial0Log, SerialLog& serial1Log,
                    std::function<void(const String&)> tty0Callback, std::function<void(const String&)> tty1Callback);
    ~WebConfigServer();

    void setup();

    void setWiFiConfig(const String& ssid, const String& password, const String& deviceName,
                      const String& mqttBroker, int mqttPort, const String& mqttUser,
                      const String& mqttPassword);

    void setAPMode(bool apMode);
    void setAPIP(const IPAddress& ip);

private:
    PreferencesStorage& preferencesStorage;
    SerialLog& serial0Log;
    SerialLog& serial1Log;
    std::function<void(const String&)> tty0Callback;
    std::function<void(const String&)> tty1Callback;

    AsyncWebServer* server;
    bool apMode;
    IPAddress apIP;

    String processor(const String& var);
    String escapeHTML(const String& str);
};

}  // namespace jrb::wifi_serial

