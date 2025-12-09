#pragma once

#include "interfaces/iwifi_manager.h"
#include <WiFi.h>
#include <Preferences.h>

namespace jrb::wifi_serial {

class WiFiManager final : public IWiFiManager {
public:
    WiFiManager();
    ~WiFiManager();
    
    void begin(::Preferences* prefs) override;
    bool connect() override;
    void loop() override;
    
    String getSSID() const override { return ssid; }
    String getPassword() const override { return password; }
    String getDeviceName() const override { return deviceName; }
    String getMqttBroker() const override { return mqttBroker; }
    int getMqttPort() const override { return mqttPort; }
    String getMqttUser() const override { return mqttUser; }
    String getMqttPassword() const override { return mqttPassword; }
    
    bool isAPMode() const override { return apMode; }
    IPAddress getAPIP() const override { return apIP; }
    void resetSettings() override;

private:
    ::Preferences* preferences;
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

