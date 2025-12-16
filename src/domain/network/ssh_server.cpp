#include "ssh_server.h"
#include "domain/config/preferences_storage.h"
#include "libssh_esp32.h"
#include "system_info.h"
#include <ArduinoLog.h>
#include <WiFi.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <nonstd/span.hpp>

namespace jrb::wifi_serial {

SSHServer::SSHServer(PreferencesStorage &storage, SystemInfo &sysInfo,
                     SpecialCharacterHandler &specialCharacterHandler)
    : preferencesStorage(storage), systemInfo(sysInfo), sshBind(nullptr),
      running(false), sshTaskHandle(nullptr), serialWrite(nullptr),
      serialToSSHQueue(nullptr), activeSSHSession(false),
      specialCharacterHandler(specialCharacterHandler) {
  Log.traceln(__PRETTY_FUNCTION__);

  // Create FreeRTOS queue for thread-safe serial→SSH data transfer
  serialToSSHQueue = xQueueCreate(SSH_QUEUE_SIZE, SSH_QUEUE_ITEM_SIZE);
  if (!serialToSSHQueue) {
    Log.errorln("SSH: Failed to create serialToSSH queue");
  } else {
    Log.infoln("SSH: Queue created successfully (size=%d, item_size=%d)",
               SSH_QUEUE_SIZE, SSH_QUEUE_ITEM_SIZE);
  }
}

SSHServer::~SSHServer() {
  stop();
  if (sshBind) {
    ssh_bind_free((ssh_bind)sshBind);
  }
  if (serialToSSHQueue) {
    vQueueDelete(serialToSSHQueue);
    serialToSSHQueue = nullptr;
  }
}

void SSHServer::setSerialWriteCallback(SerialWriteCallback writeCallback) {
  serialWrite = writeCallback;
}

void SSHServer::sendToSSHClients(const nonstd::span<const uint8_t> &data) {
  if (!running || !serialToSSHQueue || data.empty() || !activeSSHSession)
    return;

  // Prepare data for queue (fixed-size buffer)
  uint8_t queueItem[SSH_QUEUE_ITEM_SIZE] = {0};
  size_t copySize =
      data.size() < SSH_QUEUE_ITEM_SIZE ? data.size() : SSH_QUEUE_ITEM_SIZE;
  memcpy(queueItem, data.data(), copySize);

  // Send to queue (non-blocking from loop context)
  if (xQueueSend(serialToSSHQueue, queueItem, 0) != pdTRUE) {
    Log.warningln("SSH: Queue full, dropping %d bytes", data.size());
  }
}

bool SSHServer::authenticateUser(const char *user, const char *password) {
  bool userMatch = preferencesStorage.webUser.equals(user);
  bool passMatch = preferencesStorage.webPassword.equals(password);

  Log.infoln("SSH auth attempt - user: %s, result: %s", user,
             (userMatch && passMatch) ? "SUCCESS" : "FAILED");
  return userMatch && passMatch;
}

void SSHServer::sendWelcomeMessage(void *channel) {
  if (!channel)
    return;

  ssh_channel chan = (ssh_channel)channel;

  const char *banner =
      "\r\n"
      "╔══════════════════════════════════════════════════════════╗\r\n"
      "║                                                          ║\r\n"
      "║          ESP32 WiFi-Serial Bridge - SSH Console          ║\r\n"
      "║                                                          ║\r\n"
      "║            Ctrl+Y to send commands to the ESP32          ║\r\n"
      "╚══════════════════════════════════════════════════════════╝\r\n"
      "\r\n";
  ssh_channel_write(chan, banner, strlen(banner));

  // Send system information
  char sysInfoBuf[512];

  snprintf(sysInfoBuf, sizeof(sysInfoBuf),
           "Device Name:    %s\r\n"
           "IP Address:     %s\r\n"
           "MAC Address:    %s\r\n"
           "MQTT Broker:    %s:%d\r\n"
           "Baud Rate TTY1: %d\r\n"
           "\r\n"
           "Connected to:   ttyS1 (UART GPIO 0/1)\r\n"
           "\r\n",
           preferencesStorage.deviceName.c_str(),
           WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(),
           preferencesStorage.mqttBroker.c_str(), preferencesStorage.mqttPort,
           preferencesStorage.baudRateTty1);

  ssh_channel_write(chan, sysInfoBuf, strlen(sysInfoBuf));
}

String SSHServer::handleSpecialCharacter(char c) {
  static bool specialCharacterMode = false;
  if (c == CMD_PREFIX) {
    specialCharacterMode = true;
    return 
    R"(Special characters for SSH interface:
    Ctrl+Y i: print system information
    Ctrl+Y x: terminate the SSH session
    Ctrl+Y n: reset the device (INMEDIATELY)
    )";
  }
  if (!specialCharacterMode)
    return "";
  specialCharacterMode = false;
  switch (c) {
  case CMD_INFO:
    return systemInfo.getWelcomeString();
  case CMD_DISCONNECT_SSH:
    return "TERMINATE";
  case CMD_RESET:
    return "RESET";
  default:
    return "Unknown special character: " + String(c);
  }
}

void SSHServer::handleSSHSession(void *session) {
  if (!session)
    return;

  ssh_session sshSession = (ssh_session)session;
  ssh_message message;
  ssh_channel channel = nullptr;
  bool authenticated = false;

  // Authentication loop
  do {
    message = ssh_message_get(sshSession);
    if (!message)
      break;

    int type = ssh_message_type(message);
    int subtype = ssh_message_subtype(message);

    if (type == SSH_REQUEST_AUTH && subtype == SSH_AUTH_METHOD_PASSWORD) {
      const char *user = ssh_message_auth_user(message);
      const char *pass = ssh_message_auth_password(message);

      if (authenticateUser(user, pass)) {
        ssh_message_auth_reply_success(message, 0);
        authenticated = true;
        ssh_message_free(message);
        break;
      }
      ssh_message_auth_set_methods(message, SSH_AUTH_METHOD_PASSWORD);
      ssh_message_reply_default(message);
    } else {
      ssh_message_reply_default(message);
    }
    ssh_message_free(message);
  } while (!authenticated);

  if (!authenticated) {
    Log.warningln("SSH: Authentication failed");
    return;
  }

  Log.infoln("SSH: User authenticated successfully");

  // Wait for channel session
  do {
    message = ssh_message_get(sshSession);
    if (!message)
      break;

    if (ssh_message_type(message) == SSH_REQUEST_CHANNEL_OPEN &&
        ssh_message_subtype(message) == SSH_CHANNEL_SESSION) {
      channel = ssh_message_channel_request_open_reply_accept(message);
      ssh_message_free(message);
      break;
    }
    ssh_message_reply_default(message);
    ssh_message_free(message);
  } while (!channel);

  if (!channel) {
    Log.errorln("SSH: No channel session requested");
    return;
  }

  Log.infoln("SSH: Channel opened");

  // Wait for PTY and shell requests
  bool gotShell = false;
  do {
    message = ssh_message_get(sshSession);
    if (!message)
      break;

    if (ssh_message_type(message) == SSH_REQUEST_CHANNEL) {
      if (ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_PTY) {
        ssh_message_channel_request_reply_success(message);
      } else if (ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_SHELL) {
        ssh_message_channel_request_reply_success(message);
        gotShell = true;
        ssh_message_free(message);
        break;
      } else {
        ssh_message_reply_default(message);
      }
    }
    ssh_message_free(message);
  } while (!gotShell);

  if (!gotShell) {
    Log.errorln("SSH: No shell requested");
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return;
  }

  Log.infoln("SSH: Shell session started");
  activeSSHSession = true;

  // Send welcome message
  sendWelcomeMessage(channel);

  // Main SSH <-> Serial bridge loop
  uint8_t sshToSerialBuffer[128];
  uint8_t serialToSSHBuffer[SSH_QUEUE_ITEM_SIZE];

  while (ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel)) {
    // SSH -> Serial (user typing into SSH goes to serial port)
    int nbytes = ssh_channel_read_nonblocking(channel, sshToSerialBuffer,
                                              sizeof(sshToSerialBuffer), 0);
    if (nbytes > 0 && serialWrite) {
      String specialCharacterResponse =
          handleSpecialCharacter(sshToSerialBuffer[0]);
      specialCharacterResponse.replace("\n", "\r\n");
      if (specialCharacterResponse == "TERMINATE") {
        ssh_channel_write(channel, "See you later alligator!...\r\n", 33);
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return;
      }
      if (specialCharacterResponse == "RESET") {
        ssh_channel_write(channel, "Bye bye, cruel world… BRB rebooting!...\r\n", 25);
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP.restart();
        return;
      }
      if (!specialCharacterResponse.isEmpty()) {
        ssh_channel_write(channel, specialCharacterResponse.c_str(),
                          specialCharacterResponse.length());
        continue;
      }
      Log.verboseln("$ssh->ttyS1$: %d bytes", nbytes);
      ssh_channel_write(channel, sshToSerialBuffer,
                        nbytes); // echo back to SSH client
      nonstd::span<const uint8_t> bufferView(sshToSerialBuffer, nbytes);
      serialWrite(bufferView);
    }

    // Serial -> SSH (data from serial port goes to SSH client)
    if (serialToSSHQueue && xQueueReceive(serialToSSHQueue, serialToSSHBuffer,
                                          pdMS_TO_TICKS(10)) == pdTRUE) {
      // Determine actual size (queue stores fixed-size items, find null
      // terminator or use max)
      size_t actualSize = 0;
      for (size_t i = 0; i < SSH_QUEUE_ITEM_SIZE; i++) {
        if (serialToSSHBuffer[i] == 0) {
          actualSize = i;
          break;
        }
      }
      if (actualSize == 0) {
        actualSize =
            SSH_QUEUE_ITEM_SIZE; // No null terminator found, use full buffer
      }

      if (actualSize > 0) {
        Log.verboseln("$ttyS1->ssh$: %d bytes", actualSize);
        ssh_channel_write(channel, serialToSSHBuffer, actualSize);
      }
    }
  }

  Log.infoln("SSH: Session ended");
  ssh_channel_close(channel);
  ssh_channel_free(channel);
}

void SSHServer::setup() {
  Log.infoln("%s: please wait this may take a while...", __PRETTY_FUNCTION__);

  if (WiFi.status() != WL_CONNECTED) {
    Log.errorln("%s: WiFi not connected, cannot setup SSH server",
                __PRETTY_FUNCTION__);
    return;
  }

  Log.infoln("%s: Initializing libssh for ESP32", __PRETTY_FUNCTION__);
  libssh_begin();

  Log.infoln("%s: Initializing libssh", __PRETTY_FUNCTION__);
  int rc = ssh_init();
  if (rc != SSH_OK) {
    Log.errorln("%s: Failed to initialize libssh: %d", __PRETTY_FUNCTION__, rc);
    return;
  }

  Log.infoln("%s: Creating SSH bind (server listener)", __PRETTY_FUNCTION__);
  sshBind = ssh_bind_new();
  if (!sshBind) {
    Log.errorln("%s: Failed to create SSH bind", __PRETTY_FUNCTION__);
    ssh_finalize();
    return;
  }

  Log.infoln("%s: Configuring SSH bind options", __PRETTY_FUNCTION__);
  ssh_bind_options_set((ssh_bind)sshBind, SSH_BIND_OPTIONS_BINDPORT_STR, "22");

  Log.infoln("%s: Generating RSA host key", __PRETTY_FUNCTION__);
  ssh_key hostkey = nullptr;
  rc = ssh_pki_generate(SSH_KEYTYPE_RSA, 2048, &hostkey);
  if (rc != SSH_OK || !hostkey) {
    Log.errorln("%s: Failed to generate RSA host key", __PRETTY_FUNCTION__);
    ssh_bind_free((ssh_bind)sshBind);
    sshBind = nullptr;
    ssh_finalize();
    return;
  }

  Log.infoln("%s: Setting host key for SSH bind", __PRETTY_FUNCTION__);
  rc = ssh_bind_options_set((ssh_bind)sshBind, SSH_BIND_OPTIONS_IMPORT_KEY,
                            hostkey);
  if (rc != SSH_OK) {
    Log.errorln("%s: Failed to set host key", __PRETTY_FUNCTION__);
    ssh_key_free(hostkey);
    ssh_bind_free((ssh_bind)sshBind);
    sshBind = nullptr;
    ssh_finalize();
    return;
  }

  Log.infoln("SSH Server: RSA host key generated successfully");

  Log.infoln("%s: Starting listening", __PRETTY_FUNCTION__);
  if (ssh_bind_listen((ssh_bind)sshBind) < 0) {
    Log.errorln("SSH Server: Failed to listen on port 22: %s",
                ssh_get_error((ssh_bind)sshBind));
    ssh_key_free(hostkey);
    ssh_bind_free((ssh_bind)sshBind);
    sshBind = nullptr;
    ssh_finalize();
    return;
  }

  running = true;
  Log.infoln("SSH Server: Started successfully on port 22");
  Log.infoln("SSH Server: Connect with: ssh %s@%s",
             preferencesStorage.webUser.c_str(),
             WiFi.localIP().toString().c_str());

  Log.infoln("%s: Starting SSH task", __PRETTY_FUNCTION__);
  xTaskCreate(sshTask, "SSH_Server",
              8192, // Stack size
              this,
              1, // Priority
              &sshTaskHandle);

  if (sshTaskHandle) {
    Log.infoln("%s: Task created successfully", __PRETTY_FUNCTION__);
  } else {
    Log.errorln("%s: Failed to create task", __PRETTY_FUNCTION__);
    running = false;
  }
}

void SSHServer::stop() {
  if (sshTaskHandle) {
    running = false;
    vTaskDelete(sshTaskHandle);
    sshTaskHandle = nullptr;
  }
}

void SSHServer::sshTask(void *parameter) {
  SSHServer *server = static_cast<SSHServer *>(parameter);
  server->runSSHTask();
}

void SSHServer::runSSHTask() {
  Log.infoln("%s: Started", __PRETTY_FUNCTION__);

  while (running) {
    // Accept new SSH connection (blocking, but in separate task)
    ssh_session newSession = ssh_new();
    if (!newSession) {
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    int rc = ssh_bind_accept((ssh_bind)sshBind, newSession);

    if (rc == SSH_OK) {
      Log.infoln("SSH Task: New connection accepted");

      // Handle key exchange
      if (ssh_handle_key_exchange(newSession) == SSH_OK) {
        Log.infoln("SSH Task: Key exchange successful");
        handleSSHSession(newSession);
      } else {
        Log.errorln("SSH Task: Key exchange failed: %s",
                    ssh_get_error(newSession));
      }

      // Clean up session
      ssh_disconnect(newSession);
      ssh_free(newSession);
    } else {
      ssh_free(newSession);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }

  Log.infoln("SSH Task: Stopped");
  vTaskDelete(nullptr);
}

bool SSHServer::isRunning() const { return running; }

} // namespace jrb::wifi_serial
