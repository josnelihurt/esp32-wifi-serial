#include "application.h"
#include "infrastructure/mqttt/mqtt_client.h"
#include <Arduino.h>
#include <ArduinoLog.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

namespace jrb::wifi_serial {

// Static instance pointer for MQTT callbacks
Application *Application::s_instance = nullptr;

Application::Application()
    : preferencesStorage(), wifiManager(preferencesStorage),
      mqttClient(wifiClient, preferencesStorage),
      systemInfo(preferencesStorage, otaEnabled),
      sshServer(preferencesStorage, systemInfo, specialCharacterHandler),
      sshSubscriber(sshServer), webServer(preferencesStorage),
      tty0Broadcaster(webServer.getTty0Stream(), mqttClient.getTty0Stream()),
      tty1Broadcaster(webServer.getTty1Stream(), mqttClient.getTty1Stream(),
                      sshSubscriber),
      otaManager(preferencesStorage, otaEnabled),
      specialCharacterHandler(systemInfo, preferencesStorage) {
  // Set static instance for MQTT callbacks
  s_instance = this;
  systemInfo.logSystemInformation();

  // Setup serial1 hardware
  setupSerial1();

  // Initialize SSH server (runs in its own FreeRTOS task)
  sshServer.setSerialWriteCallback([](const types::span<const uint8_t> &data) {
    if (s_instance->preferencesStorage.debugEnabled) {
      String dataString((const char *)data.data(), data.size());
      Log.infoln("$ssh->ttyS1$%s", dataString.c_str());
    }
    if (s_instance->serial1) {
      s_instance->serial1->write(data.data(), data.size());
    }
  });
}

Application::~Application() {}

void Application::setupSerial1() {
  int baudRate = preferencesStorage.baudRateTty1;
  if (baudRate <= 1) {
    Log.errorln("%s: baudRateTty1 is %d (<= 1), using default 115200",
                __PRETTY_FUNCTION__, baudRate);
    baudRate = DEFAULT_BAUD_RATE_TTY1;
  }
  Log.infoln("%s: Initializing serial1 with baud rate: %d, RX: %d, TX: %d, "
             "config: 0x%x",
             __PRETTY_FUNCTION__, baudRate, SERIAL1_RX_PIN, SERIAL1_TX_PIN,
             SERIAL_8N1);
  serial1 = new HardwareSerial(1);
  serial1->begin(baudRate, SERIAL_8N1, SERIAL1_RX_PIN, SERIAL1_TX_PIN);
}

void Application::setup() {
  Log.traceln(__PRETTY_FUNCTION__);

  // Network setup (from NetworkSetupTask)
  wifiManager.setup();
  otaManager.setup();
  systemInfo.logSystemInformation();

  mqttClient.setCallbacks(
      [](const types::span<const uint8_t> &data) {
        Serial.write(data.data(), data.size());
        s_instance->tty0Broadcaster.append(data);
      },
      [](const types::span<const uint8_t> &data) {
        if (s_instance->preferencesStorage.debugEnabled) {
          String dataString(data.data(), data.size());
          Log.infoln("$mqtt->ttyS1$%s", dataString.c_str());
        }
        if (s_instance->serial1) {
          s_instance->serial1->write(data.data(), data.size());
        }
        s_instance->sshServer.sendToSSHClients(data);
      });
  preferencesStorage.save();

  webServer.setWiFiConfig(
      preferencesStorage.ssid, preferencesStorage.password,
      preferencesStorage.deviceName, preferencesStorage.mqttBroker,
      preferencesStorage.mqttPort, preferencesStorage.mqttUser,
      preferencesStorage.mqttPassword);
  webServer.setAPMode(wifiManager.isAPMode());
  if (wifiManager.isAPMode()) {
    webServer.setAPIP(wifiManager.getAPIP());
  }
  webServer.setup(
      [](const types::span<const uint8_t> &data) {
        // Handle web to serial and mqtt
        if (s_instance->preferencesStorage.debugEnabled) {
          String dataString(data.data(), data.size());
          Log.info("$web->ttyS0$%s", dataString.c_str());
        }
        s_instance->mqttClient.appendToTty0Buffer(data);
      },
      [](const types::span<const uint8_t> &data) {
        // Handle web to serial and mqtt
        if (s_instance->preferencesStorage.debugEnabled) {
          String dataString(data.data(), data.size());
          Log.info("$web->ttyS1$%s", dataString.c_str());
        }
        s_instance->mqttClient.appendToTty1Buffer(data);
        if (s_instance->serial1) {
          s_instance->serial1->write(data.data(), data.size());
        }
        s_instance->sshServer.sendToSSHClients(data);
      });

  // SSH server setup (after network is ready)
  sshServer.setup();

  lastInfoPublish = millis();
  systemInfo.logSystemInformation();
  Log.infoln("Setup complete!");
}

void Application::loop() {
  if (buttonHandler.checkTriplePress()) {
    Log.infoln(
        "Triple press detected! Resetting configuration and restarting...");
    preferencesStorage.clear();
    ESP.restart();
  }

  wifiManager.loop();
  mqttClient.loop();
  ArduinoOTA.handle();

  reconnectMqttIfNeeded();
  publishInfoIfNeeded();

  handleSerialPort0();
  handleSerialPort1();
}

void Application::reconnectMqttIfNeeded() {
  if (!wifiManager.isAPMode() && preferencesStorage.mqttBroker.length() > 0 &&
      !mqttClient.isConnected()) {
    unsigned long now = millis();
    if (now - lastMqttReconnectAttempt >= 5000) {
      lastMqttReconnectAttempt = now;
      const char *user = preferencesStorage.mqttUser.length() > 0
                             ? preferencesStorage.mqttUser.c_str()
                             : nullptr;
      const char *pass = preferencesStorage.mqttPassword.length() > 0
                             ? preferencesStorage.mqttPassword.c_str()
                             : nullptr;
      mqttClient.connect(preferencesStorage.mqttBroker.c_str(),
                         preferencesStorage.mqttPort, user, pass);
    }
  }
}

void Application::publishInfoIfNeeded() {
  if (!wifiManager.isAPMode() && mqttClient.isConnected()) {
    static constexpr unsigned long INFO_PUBLISH_INTERVAL_MS = 30000;

    if (millis() - lastInfoPublish >= INFO_PUBLISH_INTERVAL_MS) {
      types::string macAddress = WiFi.macAddress().c_str();
      types::string ipAddress = WiFi.status() == WL_CONNECTED
                                    ? WiFi.localIP().toString().c_str()
                                    : "Not connected";
      types::string ssid = WiFi.status() == WL_CONNECTED ? WiFi.SSID().c_str()
                                                         : "Not configured";
      mqttClient.publishInfo(
          preferencesStorage.serialize(ipAddress, macAddress, ssid));
      lastInfoPublish = millis();
    }
  }
}

void Application::handleSerialPort0() {
  while (Serial.available() > 0) {
    uint8_t byte = Serial.read();

    // Handle special commands BEFORE broadcasting
    if (specialCharacterHandler.handle(byte)) {
      continue;
    }

    // Local echo (if debug enabled)
    if (preferencesStorage.debugEnabled) {
      Serial.write(byte);
    }
    if (preferencesStorage.tty02tty1Bridge) {
      writeToSerial1(byte);
    }
    // Broadcast to all tty0 subscribers via type-erased broadcaster
    tty0Broadcaster.append(byte);
  }
}

void Application::handleSerialPort1() {
  if (serial1 == nullptr) {
    Log.errorln("%s: serial1 is nullptr", __PRETTY_FUNCTION__);
    return;
  }

  // Read from hardware Serial1 and broadcast to all subscribers
  while (serial1->available() > 0) {
    uint8_t byte = serial1->read();

    // Local echo (if debug enabled)
    if (preferencesStorage.tty02tty1Bridge) {
      Serial.write(byte);
    }

    // Broadcast to all tty1 subscribers via type-erased broadcaster
    tty1Broadcaster.append(byte);
  }
}

void Application::writeToSerial1(uint8_t byte) {
  if (serial1 == nullptr) {
    Log.errorln("%s: serial1 is nullptr", __PRETTY_FUNCTION__);
    return;
  }
  serial1->write(byte);
}
} // namespace jrb::wifi_serial
