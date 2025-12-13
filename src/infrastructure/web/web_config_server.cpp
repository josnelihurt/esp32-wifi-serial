#include "web_config_server.h"
#include "domain/config/preferences_storage.h"
#include "domain/serial/serial_log.h"
#include <Arduino.h>
#include <ArduinoLog.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WiFi.h>
namespace jrb::wifi_serial {

WebConfigServer::WebConfigServer(PreferencesStorage &storage,
                                 SerialLog &serial0Log, SerialLog &serial1Log,
                                 SerialSendCallback sendCallback)
    : preferencesStorage(storage), serial0Log(serial0Log),
      serial1Log(serial1Log), sendCallback(sendCallback), server(nullptr),
      apMode(false) {
  Log.traceln(__PRETTY_FUNCTION__);
}

WebConfigServer::~WebConfigServer() {
  if (server) {
    delete server;
  }
}

void WebConfigServer::setWiFiConfig(const String &ssid, const String &password,
                                    const String &deviceName,
                                    const String &mqttBroker, int mqttPort,
                                    const String &mqttUser,
                                    const String &mqttPassword) {}

void WebConfigServer::setAPMode(bool apMode) {
  Log.infoln(__PRETTY_FUNCTION__, "apMode: %s", apMode ? "true" : "false");
  this->apMode = apMode;
}

void WebConfigServer::setAPIP(const IPAddress &ip) {
  Log.infoln(__PRETTY_FUNCTION__, "ip: %s", ip.toString().c_str());
  this->apIP = ip;
}

void WebConfigServer::setup() {
  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    Log.errorln("LittleFS mount failed");
    return;
  }
  Log.infoln("LittleFS mounted successfully");

  // Stop and delete old server if exists
  if (server) {
    delete server;
    server = nullptr;
  }

  Log.infoln(__PRETTY_FUNCTION__, "Creating async web server");
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

  // Serve OTA HTML with template processor
  server->on("/ota.html", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!request->authenticate(preferencesStorage.webUser.c_str(),
                               preferencesStorage.webPassword.c_str())) {
      return request->requestAuthentication();
    }
    Log.traceln("%s: Handling /ota.html request", __PRETTY_FUNCTION__);
    request->send(LittleFS, "/ota.html", "text/html", false,
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
        preferencesStorage.baudRateTty1 = 115200;
      } else {
        preferencesStorage.baudRateTty1 = baudRate;
      }
    }

    // Process WiFi settings
    if (request->hasParam("ssid", true)) {
      preferencesStorage.ssid = request->getParam("ssid", true)->value();
    }
    if (request->hasParam("password", true)) {
      String newPassword = request->getParam("password", true)->value();
      if (newPassword.length() > 0 && newPassword != "********") {
        preferencesStorage.password = newPassword;
      }
    }

    // Process device name and MQTT topics
    if (request->hasParam("device", true)) {
      preferencesStorage.deviceName =
          request->getParam("device", true)->value();
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
          request->getParam("broker", true)->value();
    }
    if (request->hasParam("port", true)) {
      preferencesStorage.mqttPort =
          request->getParam("port", true)->value().toInt();
    }
    if (request->hasParam("user", true)) {
      preferencesStorage.mqttUser = request->getParam("user", true)->value();
    }
    if (request->hasParam("mqttpass", true)) {
      String newMqttPassword = request->getParam("mqttpass", true)->value();
      if (newMqttPassword.length() > 0 && newMqttPassword != "********") {
        preferencesStorage.mqttPassword = newMqttPassword;
      }
    }

    // Process Web User settings
    if (request->hasParam("web_user", true)) {
      preferencesStorage.webUser = request->getParam("web_user", true)->value();
    }
    if (request->hasParam("web_password", true)) {
      String newWebPassword = request->getParam("web_password", true)->value();
      if (newWebPassword.length() > 0 && newWebPassword != "********") {
        preferencesStorage.webPassword = newWebPassword;
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
    if (!request->authenticate(preferencesStorage.webUser.c_str(),
                               preferencesStorage.webPassword.c_str())) {
      return request->requestAuthentication();
    }
    Log.traceln("%s: Handling /serial0/poll request", __PRETTY_FUNCTION__);
    static int lastPos0 = 0;
    String newData = serial0Log.getNewData(lastPos0);
    request->send(200, "text/plain", newData);
  });

  // Serial1 polling - simplified for async
  server->on("/serial1/poll", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!request->authenticate(preferencesStorage.webUser.c_str(),
                               preferencesStorage.webPassword.c_str())) {
      return request->requestAuthentication();
    }
    Log.traceln("%s: Handling /serial1/poll request", __PRETTY_FUNCTION__);
    static int lastPos1 = 0;
    String newData = serial1Log.getNewData(lastPos1);
    request->send(200, "text/plain", newData);
  });

  // Serial0 send
  server->on(
      "/serial0/send", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (!request->authenticate(preferencesStorage.webUser.c_str(),
                                   preferencesStorage.webPassword.c_str())) {
          return request->requestAuthentication();
        }
        Log.traceln("%s: Handling /serial0/send request", __PRETTY_FUNCTION__);
        if (!request->hasParam("data", true)) {
          request->send(400, "text/plain", "Missing data");
          return;
        }
        String data = request->getParam("data", true)->value();
        if (sendCallback) {
          sendCallback(0, data);
        }
        request->send(200, "text/plain", "OK");
      });

  // Serial1 send
  server->on(
      "/serial1/send", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (!request->authenticate(preferencesStorage.webUser.c_str(),
                                   preferencesStorage.webPassword.c_str())) {
          return request->requestAuthentication();
        }
        Log.traceln("%s: Handling /serial1/send request", __PRETTY_FUNCTION__);
        if (!request->hasParam("data", true)) {
          request->send(400, "text/plain", "Missing data");
          return;
        }
        String data = request->getParam("data", true)->value();
        if (sendCallback) {
          sendCallback(1, data);
        }
        request->send(200, "text/plain", "OK");
      });

  server->begin();
  Log.infoln("Async web server started");
}

String WebConfigServer::processor(const String &var) {
  // This function is called for each %VARIABLE% in the HTML template

  if (var == "SSID") {
    return escapeHTML(preferencesStorage.ssid);
  }
  if (var == "PASSWORD_DISPLAY") {
    return preferencesStorage.password.length() > 0 ? "********" : "";
  }
  if (var == "PASSWORD_HAS_VALUE") {
    return preferencesStorage.password.length() > 0 ? "1" : "0";
  }
  if (var == "DEVICE_NAME") {
    return escapeHTML(preferencesStorage.deviceName.length() > 0
                          ? preferencesStorage.deviceName
                          : "esp32c3");
  }
  if (var == "MQTT_BROKER") {
    return escapeHTML(preferencesStorage.mqttBroker);
  }
  if (var == "MQTT_PORT") {
    return String(preferencesStorage.mqttPort > 0 ? preferencesStorage.mqttPort
                                                  : 1883);
  }
  if (var == "MQTT_USER") {
    return escapeHTML(preferencesStorage.mqttUser);
  }
  if (var == "MQTT_PASSWORD_DISPLAY") {
    return preferencesStorage.mqttPassword.length() > 0 ? "********" : "";
  }
  if (var == "MQTT_PASSWORD_HAS_VALUE") {
    return preferencesStorage.mqttPassword.length() > 0 ? "1" : "0";
  }
  if (var == "TOPIC_TTY0_RX") {
    return escapeHTML(preferencesStorage.topicTty0Rx);
  }
  if (var == "TOPIC_TTY0_TX") {
    return escapeHTML(preferencesStorage.topicTty0Tx);
  }
  if (var == "TOPIC_TTY1_RX") {
    return escapeHTML(preferencesStorage.topicTty1Rx);
  }
  if (var == "TOPIC_TTY1_TX") {
    return escapeHTML(preferencesStorage.topicTty1Tx);
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
    return escapeHTML(preferencesStorage.webUser);
  }
  if (var == "WEB_PASSWORD_DISPLAY") {
    return preferencesStorage.webPassword.length() > 0 ? "********" : "";
  }
  if (var == "WEB_PASSWORD_HAS_VALUE") {
    return preferencesStorage.webPassword.length() > 0 ? "1" : "0";
  }

  return String(); // Return empty string for unknown variables
}

String WebConfigServer::escapeHTML(const String &str) {
  String escaped = str;
  escaped.replace("&", "&amp;");
  escaped.replace("<", "&lt;");
  escaped.replace(">", "&gt;");
  escaped.replace("\"", "&quot;");
  escaped.replace("'", "&#39;");
  return escaped;
}

} // namespace jrb::wifi_serial
