#pragma once

#include "domain/config/preferences_storage.h"
#include "domain/serial/serial_log.hpp"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <functional>
#include <nonstd/span.hpp>

namespace jrb::wifi_serial {

class WebConfigServer final {
public:
  // Function pointer old school way to do this for embedded systems,
  // std::function is fancy add additional overhead. I have tried using zero
  // cost abstaction applying policy based design, but it was getting messy. So
  // back to old school.
  using SerialWriteCallback = void (*)(const nonstd::span<const uint8_t> &);

  WebConfigServer(PreferencesStorage &storage);
  ~WebConfigServer();

  void setup(SerialWriteCallback tty0Callback, SerialWriteCallback tty1Callback);

  void setWiFiConfig(const String &ssid, const String &password,
                     const String &deviceName, const String &mqttBroker,
                     int mqttPort, const String &mqttUser,
                     const String &mqttPassword);

  void setAPMode(bool apMode);
  void setAPIP(const IPAddress &ip);

  SerialLog &getTty0Stream() { return serial0Log; }
  SerialLog &getTty1Stream() { return serial1Log; }

private:
  PreferencesStorage &preferencesStorage;
  SerialLog serial0Log;
  SerialLog serial1Log;
  SerialWriteCallback tty0;
  SerialWriteCallback tty1;

  AsyncWebServer *server;
  bool apMode;
  IPAddress apIP;

  String processor(const String &var);
  String escapeHTML(const String &str);
};

} // namespace jrb::wifi_serial
