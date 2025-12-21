#include "web_config_server.h"
#include "config.h"
#include "domain/config/preferences_storage.h"
#include "domain/serial/serial_log.hpp"
#include "infrastructure/logging/logger.h"
#include "infrastructure/types.hpp"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <algorithm>
#include <cstring>
#include <mbedtls/sha256.h>
namespace jrb::wifi_serial {

namespace {
void handleSerialPoll(AsyncWebServerRequest *request, SerialLog &log,
                      const PreferencesStorageDefault &prefs) {
  if (!request->authenticate(prefs.webUser.c_str(),
                             prefs.webPassword.c_str())) {
    request->requestAuthentication();
    return;
  }

  if (!log.hasData()) {
    request->send(http::toInt(http::StatusCode::OK),
                  http::toString(http::mime::TEXT_PLAIN), "");
    return;
  }

  char tmp[512];
  size_t n = log.drainTo(reinterpret_cast<uint8_t *>(tmp), sizeof(tmp) - 1);
  tmp[n] = '\0';

  request->send(200, "text/plain", tmp);
}

void handleSerialSend(AsyncWebServerRequest *request,
                      WebConfigServer::SerialWriteCallback &callback,
                      const PreferencesStorageDefault &prefs) {
  if (!request->authenticate(prefs.webUser.c_str(),
                             prefs.webPassword.c_str())) {
    return request->requestAuthentication();
  }
  if (!request->hasParam("data", true)) {
    request->send(http::toInt(http::StatusCode::BAD_REQUEST),
                  http::toString(http::mime::TEXT_PLAIN), "Missing data");
    return;
  }
  const String &data = request->getParam("data", true)->value();
  if (callback) {
    const types::span<const uint8_t> span(
        reinterpret_cast<const uint8_t *>(data.c_str()), data.length());
    callback(span);
  }
  request->send(http::toInt(http::StatusCode::OK),
                http::toString(http::mime::TEXT_PLAIN), "OK");
}

} // namespace

WebConfigServer::WebConfigServer(PreferencesStorageDefault &storage)
    : preferencesStorage(storage), serial0Log(), serial1Log(), tty0(tty0),
      tty1(tty1), server(nullptr), apMode(false), otaInProgress(false),
      otaExpectedSize(0), otaReceivedSize(0), otaExpectedHash(""),
      otaCalculatedHash(""), otaRequirePassword(false) {
#ifndef DISABLE_DEFAULT_OTA_PASSWORD
  otaRequirePassword = true;
#else
  otaRequirePassword = false;
#endif
  Log.traceln(__PRETTY_FUNCTION__);
}

WebConfigServer::~WebConfigServer() {
  if (server) {
    delete server;
  }
}

void WebConfigServer::setWiFiConfig(const types::string &ssid,
                                    const types::string &password,
                                    const types::string &deviceName,
                                    const types::string &mqttBroker,
                                    int mqttPort, const types::string &mqttUser,
                                    const types::string &mqttPassword) {}

void WebConfigServer::setAPMode(bool apMode) {
  LOG_INFO(__PRETTY_FUNCTION__, "apMode: %s", apMode ? "true" : "false");
  this->apMode = apMode;
}

void WebConfigServer::setAPIP(const IPAddress &ip) {
  LOG_INFO(__PRETTY_FUNCTION__, "ip: %s", ip.toString().c_str());
  this->apIP = ip;
}

void WebConfigServer::setup(WebConfigServer::SerialWriteCallback onTtyS0Write,
                            WebConfigServer::SerialWriteCallback onTtyS1Write) {
  // Initialize LittleFS
  tty0 = onTtyS0Write;
  tty1 = onTtyS1Write;
  if (!LittleFS.begin(true)) {
    Log.errorln("LittleFS mount failed");
    return;
  }
  LOG_INFO("LittleFS mounted successfully");

  // Stop and delete old server if exists
  if (server) {
    delete server;
    server = nullptr;
  }

  LOG_INFO(__PRETTY_FUNCTION__, "Creating async web server");
  server = new AsyncWebServer(80);

  // Serve index.html with template processor
  server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!request->authenticate(preferencesStorage.webUser.c_str(),
                               preferencesStorage.webPassword.c_str())) {
      return request->requestAuthentication();
    }
    Log.traceln("%s: Handling / request", __PRETTY_FUNCTION__);
    request->send(LittleFS, "/index.html", "text/html", false,
                  [this](const String &var) { return this->processor(var); });
  });

  // Serve static CSS file (no template processing)
  server->on("/style.css", HTTP_GET, [this](AsyncWebServerRequest *request) {
    Log.traceln("%s: Handling /style.css request", __PRETTY_FUNCTION__);
    request->send(LittleFS, "/style.css", "text/css");
  });

  // Serve static JavaScript file (no template processing)
  server->on("/script.js", HTTP_GET, [this](AsyncWebServerRequest *request) {
    Log.traceln("%s: Handling /script.js request", __PRETTY_FUNCTION__);
    request->send(LittleFS, "/script.js", "application/javascript");
  });
  server->on("/favicon.svg", HTTP_GET, [this](AsyncWebServerRequest *request) {
    Log.traceln("%s: Handling /favicon.svg request", __PRETTY_FUNCTION__);
    request->send(LittleFS, "/favicon.svg", "image/svg+xml");
  });

  // Serve OTA Firmware HTML with template processor
  server->on("/ota.html", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!request->authenticate(preferencesStorage.webUser.c_str(),
                               preferencesStorage.webPassword.c_str())) {
      return request->requestAuthentication();
    }
    Log.traceln("%s: Handling /ota.html request", __PRETTY_FUNCTION__);
    request->send(LittleFS, "/ota.html", "text/html", false,
                  [this](const String &var) { return this->processor(var); });
  });

  // Serve OTA Filesystem HTML with template processor
  server->on("/ota-fs.html", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!request->authenticate(preferencesStorage.webUser.c_str(),
                               preferencesStorage.webPassword.c_str())) {
      return request->requestAuthentication();
    }
    Log.traceln("%s: Handling /ota-fs.html request", __PRETTY_FUNCTION__);
    request->send(LittleFS, "/ota-fs.html", "text/html", false,
                  [this](const String &var) { return this->processor(var); });
  });

  // Serve About HTML with template processor
  server->on("/about.html", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!request->authenticate(preferencesStorage.webUser.c_str(),
                               preferencesStorage.webPassword.c_str())) {
      return request->requestAuthentication();
    }
    Log.traceln("%s: Handling /about.html request", __PRETTY_FUNCTION__);
    request->send(LittleFS, "/about.html", "text/html", false,
                  [this](const String &var) { return this->processor(var); });
  });

  // Handle configuration save
  server->on("/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
    if (!request->authenticate(preferencesStorage.webUser.c_str(),
                               preferencesStorage.webPassword.c_str())) {
      return request->requestAuthentication();
    }
    Log.traceln("%s: Handling /save request", __PRETTY_FUNCTION__);

    // Process baud rate
    if (request->hasParam("speed0", true)) {
      int baudRate = request->getParam("speed0", true)->value().toInt();
      if (baudRate <= 1) {
        Log.errorln("Invalid baud rate %d (must be > 1), using default 115200",
                    baudRate);
        preferencesStorage.baudRateTty1 = DEFAULT_BAUD_RATE_TTY1;
      } else {
        preferencesStorage.baudRateTty1 = baudRate;
      }
    }

    // Process WiFi settings
    if (request->hasParam("ssid", true)) {
      preferencesStorage.ssid =
          request->getParam("ssid", true)->value().c_str();
    }
    if (request->hasParam("password", true)) {
      String newPassword = request->getParam("password", true)->value();
      if (newPassword.length() > 0 && newPassword != "********") {
        preferencesStorage.password = newPassword.c_str();
      }
    }

    // Process device name and MQTT topics
    if (request->hasParam("device", true)) {
      preferencesStorage.deviceName =
          request->getParam("device", true)->value().c_str();
      preferencesStorage.topicTty0Rx =
          "wifi_serial/" + preferencesStorage.deviceName + "/ttyS0/rx";
      preferencesStorage.topicTty0Tx =
          "wifi_serial/" + preferencesStorage.deviceName + "/ttyS0/tx";
      preferencesStorage.topicTty1Rx =
          "wifi_serial/" + preferencesStorage.deviceName + "/ttyS1/rx";
      preferencesStorage.topicTty1Tx =
          "wifi_serial/" + preferencesStorage.deviceName + "/ttyS1/tx";
    }

    // Process MQTT settings
    if (request->hasParam("broker", true)) {
      preferencesStorage.mqttBroker =
          request->getParam("broker", true)->value().c_str();
    }
    if (request->hasParam("port", true)) {
      preferencesStorage.mqttPort =
          request->getParam("port", true)->value().toInt();
    }
    if (request->hasParam("user", true)) {
      preferencesStorage.mqttUser =
          request->getParam("user", true)->value().c_str();
    }
    if (request->hasParam("mqttpass", true)) {
      String newMqttPassword = request->getParam("mqttpass", true)->value();
      if (newMqttPassword.length() > 0 && newMqttPassword != "********") {
        preferencesStorage.mqttPassword = newMqttPassword.c_str();
      }
    }

    // Process Web User settings
    if (request->hasParam("web_user", true)) {
      preferencesStorage.webUser =
          request->getParam("web_user", true)->value().c_str();
    }
    if (request->hasParam("web_password", true)) {
      String newWebPassword = request->getParam("web_password", true)->value();
      if (newWebPassword.length() > 0 && newWebPassword != "********") {
        preferencesStorage.webPassword = newWebPassword.c_str();
      }
    }

    preferencesStorage.save();
    request->send(200, "text/plain", "Configuration saved! Restarting...");
    delay(1000);
    ESP.restart();
  });

  // Handle device reset
  server->on("/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
    if (!request->authenticate(preferencesStorage.webUser.c_str(),
                               preferencesStorage.webPassword.c_str())) {
      return request->requestAuthentication();
    }
    Log.traceln("%s: Handling /reset request", __PRETTY_FUNCTION__);
    request->send(200, "text/plain", "Device resetting...");
    delay(500);
    ESP.restart();
  });

  // Serial0 polling - simplified for async
  server->on("/serial0/poll", HTTP_GET, [this](AsyncWebServerRequest *request) {
    Log.traceln("%s: Handling /serial0/poll request", __PRETTY_FUNCTION__);
    handleSerialPoll(request, serial0Log, preferencesStorage);
  });

  // Serial1 polling - simplified for async
  server->on("/serial1/poll", HTTP_GET, [this](AsyncWebServerRequest *request) {
    Log.traceln("%s: Handling /serial1/poll request", __PRETTY_FUNCTION__);
    handleSerialPoll(request, serial1Log, preferencesStorage);
  });

  // Serial0 send
  server->on("/serial0/send", HTTP_POST, [&](AsyncWebServerRequest *request) {
    Log.traceln("%s: Handling /serial0/send request", __PRETTY_FUNCTION__);
    handleSerialSend(request, tty0, preferencesStorage);
  });

  // Serial1 send
  server->on("/serial1/send", HTTP_POST, [&](AsyncWebServerRequest *request) {
    Log.traceln("%s: Handling /serial1/send request", __PRETTY_FUNCTION__);
    handleSerialSend(request, tty1, preferencesStorage);
  });

  // Setup OTA endpoints
  setupOTAEndpoints();

  server->begin();
  LOG_INFO("Async web server started");
}

String WebConfigServer::processor(const String &var) {
  // This function is called for each %VARIABLE% in the HTML template

  if (var == "SSID") {
    return String(escapeHTML(preferencesStorage.ssid).c_str());
  }
  if (var == "PASSWORD_DISPLAY") {
    return preferencesStorage.password.length() > 0 ? "********" : "";
  }
  if (var == "PASSWORD_HAS_VALUE") {
    return preferencesStorage.password.length() > 0 ? "1" : "0";
  }
  if (var == "DEVICE_NAME") {
    return String(escapeHTML(preferencesStorage.deviceName.length() > 0
                                 ? preferencesStorage.deviceName
                                 : "esp32c3")
                      .c_str());
  }
  if (var == "MQTT_BROKER") {
    return String(escapeHTML(preferencesStorage.mqttBroker).c_str());
  }
  if (var == "MQTT_PORT") {
    return String(preferencesStorage.mqttPort > 0 ? preferencesStorage.mqttPort
                                                  : 1883);
  }
  if (var == "MQTT_USER") {
    return String(escapeHTML(preferencesStorage.mqttUser).c_str());
  }
  if (var == "MQTT_PASSWORD_DISPLAY") {
    return preferencesStorage.mqttPassword.length() > 0 ? "********" : "";
  }
  if (var == "MQTT_PASSWORD_HAS_VALUE") {
    return preferencesStorage.mqttPassword.length() > 0 ? "1" : "0";
  }
  if (var == "TOPIC_TTY0_RX") {
    return String(escapeHTML(preferencesStorage.topicTty0Rx).c_str());
  }
  if (var == "TOPIC_TTY0_TX") {
    return String(escapeHTML(preferencesStorage.topicTty0Tx).c_str());
  }
  if (var == "TOPIC_TTY1_RX") {
    return String(escapeHTML(preferencesStorage.topicTty1Rx).c_str());
  }
  if (var == "TOPIC_TTY1_TX") {
    return String(escapeHTML(preferencesStorage.topicTty1Tx).c_str());
  }
  if (var == "BAUD_RATE_TTY1") {
    return String(preferencesStorage.baudRateTty1);
  }
  if (var == "IP_ADDRESS") {
    return (apMode ? apIP.toString() : WiFi.localIP().toString());
  }
  if (var == "OTA_PASSWORD_STATUS") {
    return preferencesStorage.webPassword.length() > 0
               ? "Configured (uses Web Password)"
               : "Not configured - OTA is unprotected!";
  }

  if (var == "WEB_USER") {
    return String(escapeHTML(preferencesStorage.webUser).c_str());
  }
  if (var == "WEB_PASSWORD_DISPLAY") {
    return preferencesStorage.webPassword.length() > 0 ? "********" : "";
  }
  if (var == "WEB_PASSWORD_HAS_VALUE") {
    return preferencesStorage.webPassword.length() > 0 ? "1" : "0";
  }

  return String(); // Return empty string for unknown variables
}

types::string WebConfigServer::escapeHTML(const types::string &str) {
  types::string escaped = str;
  size_t pos = 0;
  // Replace & first to avoid double-escaping
  while ((pos = escaped.find("&", pos)) != types::string::npos) {
    escaped.replace(pos, 1, "&amp;");
    pos += 5;
  }
  pos = 0;
  while ((pos = escaped.find("<", pos)) != types::string::npos) {
    escaped.replace(pos, 1, "&lt;");
    pos += 4;
  }
  pos = 0;
  while ((pos = escaped.find(">", pos)) != types::string::npos) {
    escaped.replace(pos, 1, "&gt;");
    pos += 4;
  }
  pos = 0;
  while ((pos = escaped.find("\"", pos)) != types::string::npos) {
    escaped.replace(pos, 1, "&quot;");
    pos += 6;
  }
  pos = 0;
  while ((pos = escaped.find("'", pos)) != types::string::npos) {
    escaped.replace(pos, 1, "&#39;");
    pos += 5;
  }
  return escaped;
}

// OTA Web Implementation
void WebConfigServer::setupOTAEndpoints() {
  LOG_INFO("Setting up OTA endpoints");

  // Handle OTA status request
  server->on("/ota/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!request->authenticate(preferencesStorage.webUser.c_str(),
                               preferencesStorage.webPassword.c_str())) {
      return request->requestAuthentication();
    }

    String status =
        "{\"otaInProgress\":" + String(otaInProgress ? "true" : "false") +
        ",\"receivedSize\":" + String(otaReceivedSize) +
        ",\"expectedSize\":" + String(otaExpectedSize) + "}";
    request->send(200, "application/json", status);
  });

  // Handle firmware upload
  server->on(
      "/ota/firmware/upload", HTTP_POST,
      [this](AsyncWebServerRequest *request) {
        // Final response handler - called after upload completes
        if (!request->authenticate(preferencesStorage.webUser.c_str(),
                                   preferencesStorage.webPassword.c_str())) {
          return request->requestAuthentication();
        }

        if (Update.hasError()) {
          otaInProgress = false;
          request->send(500, "text/plain", "Firmware update failed");
        } else {
          LOG_INFO("Firmware update successful, restarting...");
          request->send(200, "text/plain", "Firmware updated successfully");
          delay(1000);
          ESP.restart();
        }
      },
      [this](AsyncWebServerRequest *request, String filename, size_t index,
             uint8_t *data, size_t len, bool final) {
        // File upload handler - called for each chunk
        if (!request->authenticate(preferencesStorage.webUser.c_str(),
                                   preferencesStorage.webPassword.c_str())) {
          return;
        }

#ifndef DISABLE_DEFAULT_OTA_PASSWORD
        // Require password verification
        if (index == 0 && otaRequirePassword &&
            preferencesStorage.webPassword.length() == 0) {
          Log.errorln("OTA: Password not configured");
          return;
        }
#endif

        // First chunk - initialize update
        if (index == 0) {
          LOG_INFO("Starting firmware OTA update: %s", filename.c_str());

          // Validate filename
          if (!filename.endsWith(".bin")) {
            Log.errorln("OTA: Invalid firmware file extension");
            return;
          }

          // Get expected hash from request body parameter if available
          if (request->hasParam("hash", true)) {
            otaExpectedHash = request->getParam("hash", true)->value();
            LOG_INFO("OTA: Expected hash: %s", otaExpectedHash.c_str());
          } else {
            otaExpectedHash = "";
          }

          // Initialize SHA256 context
          mbedtls_sha256_init(&sha256Ctx);
          mbedtls_sha256_starts(&sha256Ctx, 0);

          // Begin update
          if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
            Update.printError(Serial);
            Log.errorln("OTA: Update.begin() failed");
            return;
          }

          otaInProgress = true;
          otaReceivedSize = 0;
          otaExpectedSize = request->contentLength();
          LOG_INFO("OTA: Expected size: %d bytes", otaExpectedSize);
        }

        // Write chunk
        if (len) {
          if (Update.write(data, len) != len) {
            Update.printError(Serial);
            Log.errorln("OTA: Update.write() failed");
            return;
          }

          // Update hash
          mbedtls_sha256_update(&sha256Ctx, data, len);
          otaReceivedSize += len;
        }

        // Final chunk - complete update
        if (final) {
          LOG_INFO("OTA: Received %d bytes", otaReceivedSize);

          // Finalize hash
          unsigned char hash[32];
          mbedtls_sha256_finish(&sha256Ctx, hash);
          mbedtls_sha256_free(&sha256Ctx);

          // Convert hash to hex string
          char hashStr[65];
          for (int i = 0; i < 32; i++) {
            sprintf(&hashStr[i * 2], "%02x", hash[i]);
          }
          otaCalculatedHash = String(hashStr);

          LOG_INFO("OTA: Calculated hash: %s", otaCalculatedHash.c_str());

          // Verify hash if provided
          if (otaExpectedHash.length() > 0 &&
              otaCalculatedHash != otaExpectedHash) {
            Log.errorln("OTA: Hash verification failed!");
            Log.errorln("  Expected: %s", otaExpectedHash.c_str());
            Log.errorln("  Calculated: %s", otaCalculatedHash.c_str());
            Update.abort();
            otaInProgress = false;
            return;
          }

          // Complete update
          if (!Update.end(true)) {
            Update.printError(Serial);
            Log.errorln("OTA: Update.end() failed");
            otaInProgress = false;
            return;
          }

          otaInProgress = false;
          LOG_INFO("OTA: Firmware update completed successfully");
        }
      });

  // Handle filesystem upload
  server->on(
      "/ota/filesystem/upload", HTTP_POST,
      [this](AsyncWebServerRequest *request) {
        // Final response handler
        if (!request->authenticate(preferencesStorage.webUser.c_str(),
                                   preferencesStorage.webPassword.c_str())) {
          return request->requestAuthentication();
        }

        if (Update.hasError()) {
          otaInProgress = false;
          request->send(500, "text/plain", "Filesystem update failed");
        } else {
          LOG_INFO("Filesystem update successful, restarting...");
          request->send(200, "text/plain", "Filesystem updated successfully");
          delay(1000);
          ESP.restart();
        }
      },
      [this](AsyncWebServerRequest *request, String filename, size_t index,
             uint8_t *data, size_t len, bool final) {
        // File upload handler
        if (!request->authenticate(preferencesStorage.webUser.c_str(),
                                   preferencesStorage.webPassword.c_str())) {
          return;
        }

#ifndef DISABLE_DEFAULT_OTA_PASSWORD
        // Require password verification
        if (index == 0 && otaRequirePassword &&
            preferencesStorage.webPassword.length() == 0) {
          Log.errorln("OTA: Password not configured");
          return;
        }
#endif

        // First chunk - initialize update
        if (index == 0) {
          LOG_INFO("Starting filesystem OTA update: %s", filename.c_str());

          // Get expected hash from request body parameter if available
          if (request->hasParam("hash", true)) {
            otaExpectedHash = request->getParam("hash", true)->value();
            LOG_INFO("OTA: Expected hash: %s", otaExpectedHash.c_str());
          } else {
            otaExpectedHash = "";
          }

          // Initialize SHA256 context
          mbedtls_sha256_init(&sha256Ctx);
          mbedtls_sha256_starts(&sha256Ctx, 0);

          // Begin filesystem update
          // Use U_SPIFFS for LittleFS partition update
          if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS)) {
            Update.printError(Serial);
            Log.errorln("OTA: Filesystem Update.begin() failed");
            return;
          }

          otaInProgress = true;
          otaReceivedSize = 0;
          otaExpectedSize = request->contentLength();
          LOG_INFO("OTA: Expected size: %d bytes", otaExpectedSize);
        }

        // Write chunk
        if (len) {
          if (Update.write(data, len) != len) {
            Update.printError(Serial);
            Log.errorln("OTA: Filesystem Update.write() failed");
            return;
          }

          // Update hash
          mbedtls_sha256_update(&sha256Ctx, data, len);
          otaReceivedSize += len;
        }

        // Final chunk - complete update
        if (final) {
          LOG_INFO("OTA: Received %d bytes", otaReceivedSize);

          // Finalize hash
          unsigned char hash[32];
          mbedtls_sha256_finish(&sha256Ctx, hash);
          mbedtls_sha256_free(&sha256Ctx);

          // Convert hash to hex string
          char hashStr[65];
          for (int i = 0; i < 32; i++) {
            sprintf(&hashStr[i * 2], "%02x", hash[i]);
          }
          otaCalculatedHash = String(hashStr);

          LOG_INFO("OTA: Calculated hash: %s", otaCalculatedHash.c_str());

          // Verify hash if provided
          if (otaExpectedHash.length() > 0 &&
              otaCalculatedHash != otaExpectedHash) {
            Log.errorln("OTA: Hash verification failed!");
            Log.errorln("  Expected: %s", otaExpectedHash.c_str());
            Log.errorln("  Calculated: %s", otaCalculatedHash.c_str());
            Update.abort();
            otaInProgress = false;
            return;
          }

          // Complete update
          if (!Update.end(true)) {
            Update.printError(Serial);
            Log.errorln("OTA: Filesystem Update.end() failed");
            otaInProgress = false;
            return;
          }

          otaInProgress = false;
          LOG_INFO("OTA: Filesystem update completed successfully");
        }
      });
}

} // namespace jrb::wifi_serial
