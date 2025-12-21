#pragma once

#include "domain/config/preferences_storage_policy.h"

namespace jrb::wifi_serial {

class WiFiManager final {
public:
  WiFiManager(PreferencesStorageDefault &preferencesStorage);
  ~WiFiManager();

  void setup();
  bool connect();
  void loop();

  bool isAPMode() const { return apMode; }
  IPAddress getAPIP() const { return apIP; }

private:
  PreferencesStorageDefault &preferencesStorage;
  bool apMode;
  IPAddress apIP;

  void setupAP();
};

} // namespace jrb::wifi_serial
