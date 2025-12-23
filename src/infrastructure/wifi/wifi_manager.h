#pragma once

#include "domain/config/preferences_storage_policy.h"

namespace jrb::wifi_serial {

class WiFiManager final {
public:
  WiFiManager(PreferencesStorage &preferencesStorage);
  ~WiFiManager();

  void setup();
  bool connect();
  void loop();

  bool isAPMode() const { return apMode; }
  IPAddress getAPIP() const { return apIP; }

private:
  PreferencesStorage &preferencesStorage;
  bool apMode;
  IPAddress apIP;
  unsigned long apModeStartTime;

  void setupAP();
};

} // namespace jrb::wifi_serial
