#include "wifi_manager.h"
#include "config.h"

namespace jrb::wifi_serial {

WiFiManager::WiFiManager()
    : preferences{}
    , apMode{false}
    , mqttPort{DEFAULT_MQTT_PORT}
{
}

WiFiManager::~WiFiManager() {
}

void WiFiManager::begin(Preferences* prefs) {
    preferences = prefs;
    
    preferences->begin("esp32bridge", true);
    ssid = preferences->getString("wifiSSID", "");
    password = preferences->getString("wifiPassword", "");
    deviceName = preferences->getString("deviceName", DEFAULT_DEVICE_NAME);
    mqttBroker = preferences->getString("mqttBroker", "");
    mqttPort = preferences->getInt("mqttPort", DEFAULT_MQTT_PORT);
    mqttUser = preferences->getString("mqttUser", "");
    mqttPassword = preferences->getString("mqttPassword", "");
    
    // Topics are now managed by PreferencesStorage, no need to load them here
    preferences->end();
    
    if (ssid.length() == 0 || !connect()) {
        setupAP();
    }
}

bool WiFiManager::connect() {
    if (ssid.length() == 0) return false;
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
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
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    return true;
}

void WiFiManager::setupAP() {
    apMode = true;
    WiFi.mode(WIFI_AP);
    
    String macAddress = WiFi.macAddress();
    macAddress.replace(":", "");
    macAddress.toUpperCase();
    String apName = "ESP32-C3-" + macAddress.substring(6);
    
    WiFi.softAP(apName.c_str(), nullptr);
    
    apIP = WiFi.softAPIP();
    Serial.println("AP Mode");
    Serial.print("AP Name: ");
    Serial.println(apName);
    Serial.print("AP IP address: ");
    Serial.println(apIP);
}

void WiFiManager::loop() {
}

void WiFiManager::resetSettings() {
    preferences->begin("esp32bridge", false);
    preferences->clear();
    preferences->end();
    WiFi.disconnect(true);
}

}  // namespace jrb::wifi_serial

