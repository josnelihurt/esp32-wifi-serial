#pragma once

#include "domain/config/special_character_handler.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <functional>
#include <nonstd/span.hpp>

namespace jrb::wifi_serial {

class PreferencesStorage;
class SystemInfo;

/**
 * @brief SSH server that runs as a FreeRTOS task and bridges to ttyS1
 *
 * This class provides SSH access to the serial console on ttyS1, allowing
 * remote emergency access when traditional network access fails. It uses
 * the existing web credentials for authentication and runs in its own task
 * to avoid blocking the main loop.
 *
 * Uses a FreeRTOS queue for thread-safe communication of serial data from
 * the main loop to the SSH task.
 */
class SSHServer final {
public:
  using SerialWriteCallback = void (*)(const nonstd::span<const uint8_t> &);

private:
  PreferencesStorage &preferencesStorage;
  SystemInfo &systemInfo;
  void *sshBind; // Opaque pointer to libssh bind
  void *hostKey; // Opaque pointer to ssh_key (must outlive sshBind)
  bool running;
  TaskHandle_t sshTaskHandle;
  volatile bool activeSSHSession;
  bool specialCharacterMode;
  SerialWriteCallback serialWrite;
  SpecialCharacterHandler &specialCharacterHandler;

  // FreeRTOS queue for thread-safe serial→SSH data transfer
  QueueHandle_t serialToSSHQueue;
  static constexpr size_t SSH_QUEUE_SIZE = 10;
  static constexpr size_t SSH_QUEUE_PAYLOAD_SIZE = 127;
  static constexpr size_t SSH_QUEUE_ITEM_SIZE =
      SSH_QUEUE_PAYLOAD_SIZE + 1; // +1 length byte
  static constexpr int SSH_PORT = 22;
  static constexpr int SSH_RSA_KEY_BITS = 2048;
  static constexpr uint32_t SSH_TASK_STACK_SIZE = 8192;
  static constexpr UBaseType_t SSH_TASK_PRIORITY = 1;
  static constexpr TickType_t SSH_QUEUE_TIMEOUT_MS = 10;
  static constexpr uint32_t SSH_AUTH_TIMEOUT_MS = 30000;
  static constexpr uint32_t SSH_CHANNEL_TIMEOUT_MS = 10000;
  static constexpr uint32_t SSH_SHELL_TIMEOUT_MS = 10000;
  static constexpr uint32_t SSH_SESSION_TIMEOUT_MS = 3600000; // 1 hour

public:
  SSHServer(PreferencesStorage &storage, SystemInfo &sysInfo,
            SpecialCharacterHandler &specialCharacterHandler);
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
   * @brief Set callback for writing data to serial port
   *
   * @param writeCallback Function to write data to serial port
   */
  void setSerialWriteCallback(SerialWriteCallback writeCallback);

  /**
   * @brief Send data to connected SSH clients (called from main loop)
   *
   * This method is thread-safe and non-blocking. Data is queued for
   * transmission to SSH clients.
   *
   * @param data Data to send to SSH clients
   */
  void sendToSSHClients(const nonstd::span<const uint8_t> &data);

  /**
   * @brief Check if SSH server is running
   *
   * @return true if server is active and accepting connections
   */
  bool isRunning() const;

private:
  static void sshTask(void *parameter);
  void runSSHTask();
  void handleSSHSession(void *session);
  bool authenticateUser(const char *user, const char *password);
  void sendWelcomeMessage(void *channel);
  String handleSpecialCharacter(char c);
  bool authenticateSession(void *session);
  bool waitForChannelSession(void *session, void **channel);
  bool waitForShellRequest(void *session, void *channel);
};

} // namespace jrb::wifi_serial
