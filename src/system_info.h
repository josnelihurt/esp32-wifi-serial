#pragma once

#include <Arduino.h>
#include <WiFi.h>

namespace jrb::wifi_serial {

class PreferencesStorage;
class SystemInfo final {
private:
  const PreferencesStorage &preferencesStorage;
  bool &otaEnabled;

public:
  SystemInfo(const PreferencesStorage &storage, bool &ota);

  String getSpecialCharacterSettings() const;
  String getWelcomeString() const;
  void logSystemInformation() const;
};

} // namespace jrb::wifi_serial
