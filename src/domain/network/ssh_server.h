#pragma once

#include <Arduino.h>
#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

namespace jrb::wifi_serial {

class SerialBridge;
class PreferencesStorage;
class SystemInfo;

/**
 * @brief SSH server that runs as a FreeRTOS task and bridges to ttyS1
 *
 * This class provides SSH access to the serial console on ttyS1, allowing
 * remote emergency access when traditional network access fails. It uses
 * the existing web credentials for authentication and runs in its own task
 * to avoid blocking the main loop.
 */
class SSHServer final {
public:
    using SerialWriteCallback = std::function<void(const char* data, int length)>;
    using SerialReadCallback = std::function<int(char* buffer, int maxLen)>;

    SSHServer(PreferencesStorage& storage, SystemInfo& sysInfo);
    ~SSHServer();

    /**
     * @brief Initialize the SSH server and start the FreeRTOS task
     *
     * Starts listening on port 22 for incoming SSH connections
     */
    void setup();

    /**
     * @brief Stop the SSH server task
     */
    void stop();

    /**
     * @brief Set callbacks for serial communication
     *
     * @param writeCallback Function to write data to serial port
     * @param readCallback Function to read data from serial port
     */
    void setSerialCallbacks(SerialWriteCallback writeCallback, SerialReadCallback readCallback);

    /**
     * @brief Check if SSH server is running
     *
     * @return true if server is active and accepting connections
     */
    bool isRunning() const;

private:
    PreferencesStorage& preferencesStorage;
    SystemInfo& systemInfo;
    void* sshBind;     // Opaque pointer to libssh bind
    bool running;
    TaskHandle_t sshTaskHandle;

    SerialWriteCallback serialWrite;
    SerialReadCallback serialRead;

    static void sshTask(void* parameter);
    void runSSHTask();
    void handleSSHSession(void* session);
    bool authenticateUser(const char* user, const char* password);
    void sendWelcomeMessage(void* channel);
};

}  // namespace jrb::wifi_serial
