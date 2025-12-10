#pragma once

#include "dependency_container.h"
#include "task_registry.h"
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
    explicit Application(DependencyContainer& container);
    ~Application() = default;
    
    /**
     * @brief Perform early initialization.
     *
     * Called once at program start to initialize internal services and
     * prepare the application for setup. This does not start tasks that
     * require the system to be fully configured.
     */
    void initialize();

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
     * @brief Callback invoked to reset the stored configuration to defaults.
     *
     * Typically triggered by a web UI or physical action; this method
     * coordinates clearing configuration and restarting setup as needed.
     */
    void onResetConfig();

    /**
     * @brief Forward data received from the web UI to serial and MQTT.
     *
     * @param portIndex Serial port index to forward to.
     * @param data String payload coming from the web UI.
     */
    void handleWebToSerialAndMqtt(int portIndex, const String& data);
    
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

    /**
     * @brief Return a callable that triggers a configuration reset when
     *        invoked.
     */
    std::function<void()> getResetConfigCallback();

private:
    DependencyContainer& container;
    TaskRegistry registry;
    
    void registerSetupTasks();
    void registerLoopTasks();
};

}  // namespace jrb::wifi_serial

