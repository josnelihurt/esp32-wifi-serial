#pragma once

#include <WiFi.h>
#include <Preferences.h>

namespace jrb::wifi_serial {

class WiFiManager final {
public:
    WiFiManager();
    ~WiFiManager();
    
    void begin(Preferences* prefs);
    bool connect();
    void loop();
    
    String getSSID() const { return ssid; }
    String getPassword() const { return password; }
    String getDeviceName() const { return deviceName; }
    String getMqttBroker() const { return mqttBroker; }
    int getMqttPort() const { return mqttPort; }
    String getMqttUser() const { return mqttUser; }
    String getMqttPassword() const { return mqttPassword; }
    
    bool isAPMode() const { return apMode; }
    IPAddress getAPIP() const { return apIP; }
    void resetSettings();

private:
    Preferences* preferences;
    bool apMode;
    IPAddress apIP;
    String ssid;
    String password;
    String deviceName;
    String mqttBroker;
    int mqttPort;
    String mqttUser;
    String mqttPassword;
    
    void setupAP();
};

}  // namespace jrb::wifi_serial

