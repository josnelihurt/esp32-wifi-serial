#pragma once

#include "constants.h"
#include "domain/config/preferences_storage_policy.h"
#include "domain/serial/serial_log.hpp"
#include "infrastructure/types.hpp"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Update.h>
#include <functional>
#include <mbedtls/sha256.h>

namespace jrb::wifi_serial {

class WebConfigServer final {
public:
  // Function pointer old school way to do this for embedded systems,
  // std::function is fancy add additional overhead. I have tried using zero
  // cost abstaction applying policy based design, but it was getting messy. So
  // back to old school.
  using SerialWriteCallback = void (*)(const types::span<const uint8_t> &);

  // OTA Web constants
  static constexpr size_t OTA_CHUNK_SIZE = 16 * 1024;          // 16KB chunks
  static constexpr size_t MAX_FIRMWARE_SIZE = 2 * 1024 * 1024; // 2MB max
  static constexpr size_t MAX_FILESYSTEM_SIZE = 512 * 1024;    // 512KB max

  WebConfigServer(PreferencesStorage &storage);
  ~WebConfigServer() = default;

  void setup(SerialWriteCallback tty0Callback,
             SerialWriteCallback tty1Callback);

  void setWiFiConfig(const types::string &ssid, const types::string &password,
                     const types::string &deviceName,
                     const types::string &mqttBroker, int mqttPort,
                     const types::string &mqttUser,
                     const types::string &mqttPassword);

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

  AsyncWebServer server{HTTP_PORT};
  bool isServerStarted{false};
  bool apMode;
  IPAddress apIP;

  // OTA Web state
  bool otaInProgress;
  size_t otaExpectedSize;
  size_t otaReceivedSize;
  String otaExpectedHash;
  String otaCalculatedHash;
  bool otaRequirePassword;
  mbedtls_sha256_context sha256Ctx;

  String processor(const String &var);
  types::string escapeHTML(const types::string &str);

  // OTA Web methods
  void setupOTAEndpoints();
};

} // namespace jrb::wifi_serial
