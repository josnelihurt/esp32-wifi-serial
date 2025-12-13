#pragma once

#include <WiFi.h>
#include <Preferences.h>

namespace jrb::wifi_serial {
class PreferencesStorage;

class WiFiManager final {
public:
    WiFiManager(PreferencesStorage& preferencesStorage);
    ~WiFiManager();
    
    void setup();
    bool connect();
    void loop();

    bool isAPMode() const { return apMode; }
    IPAddress getAPIP() const { return apIP; }

private:
    PreferencesStorage& preferencesStorage;
    bool apMode;
    IPAddress apIP;
    
    void setupAP();
};

}  // namespace jrb::wifi_serial

