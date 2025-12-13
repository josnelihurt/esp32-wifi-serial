#pragma once

#include "interfaces/iwifi_manager.h"
#include <WiFi.h>
#include <Preferences.h>

namespace jrb::wifi_serial {
class PreferencesStorage;

class WiFiManager final : public IWiFiManager {
public:
    WiFiManager(PreferencesStorage& preferencesStorage);
    ~WiFiManager();
    
    void setup();
    bool connect() override;
    void loop() override;
    
    bool isAPMode() const override { return apMode; }
    IPAddress getAPIP() const override { return apIP; }

private:
    PreferencesStorage& preferencesStorage;
    bool apMode;
    IPAddress apIP;
    
    void setupAP();
};

}  // namespace jrb::wifi_serial

