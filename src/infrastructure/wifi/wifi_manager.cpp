#include "wifi_manager.h"
#include "config.h"
#include "domain/config/preferences_storage.h"
#include <ArduinoLog.h>
namespace jrb::wifi_serial {

WiFiManager::WiFiManager(PreferencesStorage &preferencesStorage)
    : preferencesStorage{preferencesStorage}, apMode{false} {
  Log.traceln(__PRETTY_FUNCTION__);
}

WiFiManager::~WiFiManager() {}

void WiFiManager::setup() {
  Log.traceln(__PRETTY_FUNCTION__);

  if (preferencesStorage.ssid.length() == 0 || !connect()) {
    Log.errorln("%s: %s", __PRETTY_FUNCTION__,
                "No WiFi connection found, setting up AP");
    setupAP();
  }
}

void WiFiManager::loop() {}

bool WiFiManager::connect() {
  Log.traceln(__PRETTY_FUNCTION__);
  if (preferencesStorage.ssid.length() == 0)
    return false;

  // Clean up any previous WiFi state
  WiFi.disconnect(true);
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);

  Log.infoln("%s: Attempting to connect to SSID: '%s'", __PRETTY_FUNCTION__,
             preferencesStorage.ssid.c_str());

  WiFi.begin(preferencesStorage.ssid.c_str(),
             preferencesStorage.password.c_str());

  // Wait for connection to initialize before checking status
  delay(1000);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    attempts++;

    wl_status_t status = WiFi.status();
    const char* statusStr;
    switch(status) {
      case WL_IDLE_STATUS: statusStr = "IDLE"; break;
      case WL_NO_SSID_AVAIL: statusStr = "NO_SSID_AVAIL"; break;
      case WL_SCAN_COMPLETED: statusStr = "SCAN_COMPLETED"; break;
      case WL_CONNECTED: statusStr = "CONNECTED"; break;
      case WL_CONNECT_FAILED: statusStr = "CONNECT_FAILED"; break;
      case WL_CONNECTION_LOST: statusStr = "CONNECTION_LOST"; break;
      case WL_DISCONNECTED: statusStr = "DISCONNECTED"; break;
      default: statusStr = "UNKNOWN"; break;
    }

    Log.infoln("%s: connecting to '%s' (password_len: %u), attempts: %d of "
                "30, status: %d (%s)",
                __PRETTY_FUNCTION__, preferencesStorage.ssid.c_str(),
                preferencesStorage.password.length(), attempts, status, statusStr);

    // If we see NO_SSID_AVAIL, the network isn't visible - bail out early
    if (status == WL_NO_SSID_AVAIL && attempts > 5) {
      Log.errorln("%s: SSID '%s' not found. Is it a 2.4GHz network? ESP32-C3 doesn't support 5GHz",
                  __PRETTY_FUNCTION__, preferencesStorage.ssid.c_str());
      break;
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(true);
    apMode = true;
    return false;
  }

  apMode = false;
  Log.infoln("%s: %s", __PRETTY_FUNCTION__,
             R"(WiFi connected!
IP address: %s)",
             WiFi.localIP().toString().c_str());

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
AP IP address: %s)",
             apName.c_str(), apIP.toString().c_str());
}

} // namespace jrb::wifi_serial
