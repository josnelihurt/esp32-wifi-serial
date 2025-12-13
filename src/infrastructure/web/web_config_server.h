#pragma once

#include "domain/config/preferences_storage.h"
#include "domain/serial/serial_log.h"
#include <ESPAsyncWebServer.h>
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

class WebConfigServer final {
public:
    using SerialSendCallback = std::function<void(int portIndex, const String& data)>;

    WebConfigServer(PreferencesStorage& storage, SerialLog& serial0Log, SerialLog& serial1Log,
                    SerialSendCallback sendCallback);
    ~WebConfigServer();

    void setup();
    void loop();

    void setWiFiConfig(const String& ssid, const String& password, const String& deviceName,
                      const String& mqttBroker, int mqttPort, const String& mqttUser,
                      const String& mqttPassword);

    void setAPMode(bool apMode);
    void setAPIP(const IPAddress& ip);

private:
    PreferencesStorage& preferencesStorage;
    SerialLog& serial0Log;
    SerialLog& serial1Log;
    SerialSendCallback sendCallback;

    AsyncWebServer* server;
    bool apMode;
    IPAddress apIP;

    String processor(const String& var);
    String escapeHTML(const String& str);
};

}  // namespace jrb::wifi_serial

