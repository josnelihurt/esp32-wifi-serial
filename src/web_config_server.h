#pragma once

#include "preferences_storage.h"
#include "serial_log.h"
#include <WebServer.h>
#include <DNSServer.h>
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

class WebConfigServer final {
public:
    using SerialSendCallback = std::function<void(int portIndex, const String& data)>;
    
    WebConfigServer(PreferencesStorage& storage, SerialLog& serial0Log, SerialLog& serial1Log,
                    SerialSendCallback sendCallback);
    ~WebConfigServer();
    
    void begin();
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
    
    WebServer* server;
    DNSServer* dnsServer;
    bool apMode;
    IPAddress apIP;
    
    String ssid;
    String password;
    String deviceName;
    String mqttBroker;
    int mqttPort;
    String mqttUser;
    String mqttPassword;
    
    void setupWebServer();
    String getConfigHTML();
    String escapeHTML(const String& str);
};

}  // namespace jrb::wifi_serial

