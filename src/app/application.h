#pragma once

#include "broadcaster.hpp"
#include "config.h"
#include "domain/config/preferences_storage.h"
#include "domain/config/special_character_handler.h"
#include "domain/messaging/buffered_stream.hpp"
#include "domain/messaging/mqtt_flush_policy.h"
#include "domain/network/ssh_server.h"
#include "domain/network/ssh_subscriber.h"
#include "domain/serial/serial_log.hpp"
#include "infrastructure/hardware/button_handler.h"
#include "infrastructure/mqttt/mqtt_client.h"
#include "infrastructure/web/web_config_server.h"
#include "infrastructure/wifi/wifi_manager.h"
#include "ota_manager.h"
#include "system_info.h"
#include <WiFiClient.h>
#include <functional>
#include <vector>

class HardwareSerial;
namespace jrb::wifi_serial {

/**
 * @brief Top-level application coordinator.
 *
 * The Application class owns the main wiring of the firmware: it holds a
 * reference to the shared `DependencyContainer`, registers setup and loop
 * tasks with the internal `TaskRegistry`, and exposes callbacks used by
 * external systems (MQTT, web UI, etc.) to forward incoming data into the
 * application.
 *
 * Responsibilities:
 * - Initialize dependencies and task registry.
 * - Register setup and loop tasks.
 * - Provide callbacks for MQTT tty ports and reset configuration.
 * - Bridge web/serial/mqtt messages into the appropriate handlers.
 *
 * The class is non-copyable (holds a reference to a container) and is
 * intended to be instantiated once as the program's main application object.
 */
class Application final {
public:
  Application();
  ~Application();

  /**
   * @brief Run one-time setup routines.
   *
   * Called after initialization to perform setup tasks that should run
   * once (for example, starting servers, synchronizing configuration,
   * or creating persistent resources).
   */
  void setup();

  /**
   * @brief Called continuously from the main Arduino loop.
   *
   * This drives the application's periodic processing by delegating to
   * registered loop tasks in the `TaskRegistry`.
   */
  void loop();

private:
  // Primitives (initialize first)
  bool otaEnabled{false};
  unsigned long lastInfoPublish{0};
  unsigned long lastMqttReconnectAttempt{0};

  // Stack objects (order matters - dependencies flow down)
  PreferencesStorageDefault preferencesStorage;
  WiFiManager wifiManager;
  WiFiClient wifiClient;
  MqttClient mqttClient;
  SystemInfo systemInfo;
  SSHServer sshServer;
  SSHSubscriber sshSubscriber;
  SpecialCharacterHandler specialCharacterHandler;

  WebConfigServer webServer;
  Broadcaster<SerialLog, BufferedStream<MqttFlushPolicy>> tty0Broadcaster;
  Broadcaster<SerialLog, BufferedStream<MqttFlushPolicy>, SSHSubscriber>
      tty1Broadcaster;

  // Serial hardware (heap-allocated to control initialization timing)
  HardwareSerial *serial1{nullptr};

  // Heap objects (lazy init in constructor)
  ButtonHandler buttonHandler;
  OTAManager otaManager;

  // Helper methods for setup
  void setupSerial1();

  // Helper methods for loop processing
  void handleSerialPort0();
  void handleSerialPort1();
  void reconnectMqttIfNeeded();
  void publishInfoIfNeeded();
  void writeToSerial1(uint8_t byte);

  static Application *s_instance;
};

} // namespace jrb::wifi_serial
