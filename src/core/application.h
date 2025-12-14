#pragma once

#include "domain/network/wifi_manager.h"
#include "domain/serial/serial_bridge.h"
#include "domain/serial/serial_log.h"
#include "domain/serial/serial_buffer_manager.h"
#include "domain/config/preferences_storage.h"
#include "domain/network/mqtt_client.h"
#include "domain/network/ssh_server.h"
#include "infrastructure/hardware/serial_command_handler.h"
#include "infrastructure/hardware/button_handler.h"
#include "system_info.h"
#include "ota_manager.h"
#include "infrastructure/web/web_config_server.h"
#include <WiFiClient.h>
#include <Preferences.h>
#include "config.h"
#include <Arduino.h>
#include <functional>

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
    
    /**
     * @brief Handle incoming MQTT payload for TTY port 0.
     *
     * @param data Pointer to the raw payload bytes.
     * @param length Number of bytes in the payload.
     */
    void onMqttTty0(const char* data, unsigned int length);

    /**
     * @brief Handle incoming MQTT payload for TTY port 1.
     *
     * @param data Pointer to the raw payload bytes.
     * @param length Number of bytes in the payload.
     */
    void onMqttTty1(const char* data, unsigned int length);

    
    /**
     * @brief Return a callable suitable for use as an MQTT message handler
     *        for TTY port 0.
     *
     * The returned function accepts a pointer to bytes and their length and
     * forwards the data into `onMqttTty0`.
     */
    std::function<void(const char*, unsigned int)> getMqttTty0Callback();

    /**
     * @brief Return a callable suitable for use as an MQTT message handler
     *        for TTY port 1.
     */
    std::function<void(const char*, unsigned int)> getMqttTty1Callback();


private:
    std::vector<char> tty0Buffer;
    std::vector<char> tty1Buffer;
    unsigned long tty0LastFlushMillis{0};
    unsigned long tty1LastFlushMillis{0};
    // Primitives (initialize first)
    bool otaEnabled{false};
    bool debugEnabled{true};
    unsigned long lastInfoPublish{0};
    unsigned long lastMqttReconnectAttempt{0};

    // Stack objects (order matters - dependencies flow down)
    ::Preferences preferences;
    PreferencesStorage preferencesStorage;
    WiFiManager wifiManager{preferencesStorage};
    WiFiClient wifiClient;
    MqttClient mqttClient{wifiClient};
    SerialBridge serialBridge{mqttClient};
    SerialLog serial0Log;
    SerialLog serial1Log;
    SystemInfo systemInfo{preferencesStorage, mqttClient, otaEnabled};

    // Heap objects (lazy init in constructor)
    SerialCommandHandler* serialCmdHandler{nullptr};
    ButtonHandler buttonHandler;
    OTAManager* otaManager{nullptr};
    WebConfigServer* webServer{nullptr};
    SSHServer* sshServer{nullptr};

    // Helper methods for loop processing
    void handleSerialPort0();
    void handleSerialPort1();
    bool handleSpecialCharacter(char c);
    void reconnectMqttIfNeeded();
    void publishInfoIfNeeded();
    void setupMqttCallbacks();

    // MQTT callback wrappers (need to be static for C-style function pointers)
    static void mqttTty0Wrapper(const char* data, unsigned int length);
    static void mqttTty1Wrapper(const char* data, unsigned int length);
    static Application* s_instance;
};

}  // namespace jrb::wifi_serial

