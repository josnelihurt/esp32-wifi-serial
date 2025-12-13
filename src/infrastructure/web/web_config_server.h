#pragma once

#include "interfaces/iweb_config_server.h"
#include "domain/config/preferences_storage.h"
#include "domain/serial/serial_log.h"
#include <WebServer.h>
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

class WebConfigServer final : public IWebConfigServer {
public:
    using SerialSendCallback = std::function<void(int portIndex, const String& data)>;
    
    WebConfigServer(PreferencesStorage& storage, SerialLog& serial0Log, SerialLog& serial1Log,
                    SerialSendCallback sendCallback);
    ~WebConfigServer();
    
    void begin() override;
    void loop() override;
    
    void setWiFiConfig(const String& ssid, const String& password, const String& deviceName,
                      const String& mqttBroker, int mqttPort, const String& mqttUser,
                      const String& mqttPassword) override;
    
    void setAPMode(bool apMode) override;
    void setAPIP(const IPAddress& ip) override;

private:
    PreferencesStorage& preferencesStorage;
    SerialLog& serial0Log;
    SerialLog& serial1Log;
    SerialSendCallback sendCallback;
    
    WebServer* server;
    bool apMode;
    IPAddress apIP;
    
    
    String getConfigHTML();
    String escapeHTML(const String& str);
};

}  // namespace jrb::wifi_serial

