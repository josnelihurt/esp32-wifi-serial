#include "wifi_manager.h"
#include "config.h"
#include <ArduinoLog.h>
#include "domain/config/preferences_storage.h"
namespace jrb::wifi_serial {

WiFiManager::WiFiManager(PreferencesStorage& preferencesStorage)
    : preferencesStorage{preferencesStorage}
    , apMode{false}
{   
    Log.traceln(__PRETTY_FUNCTION__);
}

WiFiManager::~WiFiManager() {
}

void WiFiManager::setup() {
    Log.traceln(__PRETTY_FUNCTION__);

    if (preferencesStorage.ssid.length() == 0 || !connect()) {
        Log.errorln("%s: %s", __PRETTY_FUNCTION__, "No WiFi connection found, setting up AP");
        setupAP();
    }
}

void WiFiManager::loop() {
}

bool WiFiManager::connect() {
    Log.traceln(__PRETTY_FUNCTION__);
    if (preferencesStorage.ssid.length() == 0) return false;
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(preferencesStorage.ssid.c_str(), preferencesStorage.password.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        apMode = true;
        return false;
    }
    
    apMode = false;
    Log.infoln("%s: %s", __PRETTY_FUNCTION__, 
R"(WiFi connected!
IP address: %s)", WiFi.localIP().toString().c_str());
    
    return true;
}

void WiFiManager::setupAP() {
    Log.traceln(__PRETTY_FUNCTION__);
    apMode = true;
    WiFi.mode(WIFI_AP);
    
    String macAddress = WiFi.macAddress();
    macAddress.replace(":", "");
    macAddress.toUpperCase();
    String apName = "ESP32-C3-" + macAddress.substring(6);
    
    WiFi.softAP(apName.c_str(), nullptr);
    
    apIP = WiFi.softAPIP();
    Log.infoln("%s: %s", __PRETTY_FUNCTION__, 
R"(AP Mode
AP Name: %s
AP IP address: %s)", apName.c_str(), apIP.toString().c_str());
}


}  // namespace jrb::wifi_serial

