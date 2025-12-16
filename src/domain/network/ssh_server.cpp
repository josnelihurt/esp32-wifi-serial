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
      hostKey(nullptr), running(false), sshTaskHandle(nullptr),
      serialWrite(nullptr), serialToSSHQueue(nullptr), activeSSHSession(false),
      specialCharacterMode(false),
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
    sshBind = nullptr;
  }
  // Free hostKey AFTER freeing sshBind (sshBind may reference it)
  if (hostKey) {
    ssh_key_free((ssh_key)hostKey);
    hostKey = nullptr;
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

  size_t copySize = data.size() < SSH_QUEUE_PAYLOAD_SIZE
                        ? data.size()
                        : SSH_QUEUE_PAYLOAD_SIZE;
  if (copySize < data.size()) {
    Log.warningln("SSH: Truncating %d bytes to %d", (int)data.size(),
                  (int)copySize);
  }

  uint8_t queueItem[SSH_QUEUE_ITEM_SIZE] = {0};
  queueItem[0] = static_cast<uint8_t>(copySize);
  memcpy(queueItem + 1, data.data(), copySize);

  if (xQueueSend(serialToSSHQueue, queueItem, 0) != pdTRUE) {
    Log.warningln("SSH: Queue full, dropping %d bytes", (int)data.size());
  }
}

bool SSHServer::authenticateUser(const char *user, const char *password) {
  if (!user || !password)
    return false;

  bool userMatch = preferencesStorage.webUser.equals(user);
  bool passMatch = preferencesStorage.webPassword.equals(password);
  bool success = userMatch && passMatch;

  Log.infoln("SSH auth attempt - user: %s, result: %s", user,
             success ? "SUCCESS" : "FAILED");
  return success;
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

bool SSHServer::authenticateSession(void *session) {
  ssh_session sshSession = (ssh_session)session;
  ssh_message message;
  uint32_t startTime = millis();

  while (millis() - startTime < SSH_AUTH_TIMEOUT_MS) {
    message = ssh_message_get(sshSession);
    if (!message) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    int type = ssh_message_type(message);
    int subtype = ssh_message_subtype(message);

    if (type == SSH_REQUEST_AUTH && subtype == SSH_AUTH_METHOD_PASSWORD) {
      const char *user = ssh_message_auth_user(message);
      const char *pass = ssh_message_auth_password(message);

      if (authenticateUser(user, pass)) {
        ssh_message_auth_reply_success(message, 0);
        ssh_message_free(message);
        return true;
      }
      ssh_message_auth_set_methods(message, SSH_AUTH_METHOD_PASSWORD);
      ssh_message_reply_default(message);
    } else {
      ssh_message_reply_default(message);
    }
    ssh_message_free(message);
  }

  Log.warningln("SSH: Authentication timeout after %d ms", SSH_AUTH_TIMEOUT_MS);
  return false;
}

bool SSHServer::waitForChannelSession(void *session, void **channel) {
  ssh_session sshSession = (ssh_session)session;
  ssh_message message;
  uint32_t startTime = millis();

  while (millis() - startTime < SSH_CHANNEL_TIMEOUT_MS) {
    message = ssh_message_get(sshSession);
    if (!message) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    if (ssh_message_type(message) == SSH_REQUEST_CHANNEL_OPEN &&
        ssh_message_subtype(message) == SSH_CHANNEL_SESSION) {
      *channel = ssh_message_channel_request_open_reply_accept(message);
      ssh_message_free(message);
      return true;
    }
    ssh_message_reply_default(message);
    ssh_message_free(message);
  }

  Log.warningln("SSH: Channel session timeout after %d ms",
                SSH_CHANNEL_TIMEOUT_MS);
  return false;
}

bool SSHServer::waitForShellRequest(void *session, void *channel) {
  ssh_session sshSession = (ssh_session)session;
  ssh_message message;
  uint32_t startTime = millis();

  while (millis() - startTime < SSH_SHELL_TIMEOUT_MS) {
    message = ssh_message_get(sshSession);
    if (!message) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    if (ssh_message_type(message) == SSH_REQUEST_CHANNEL) {
      if (ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_PTY) {
        ssh_message_channel_request_reply_success(message);
      } else if (ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_SHELL) {
        ssh_message_channel_request_reply_success(message);
        ssh_message_free(message);
        return true;
      } else {
        ssh_message_reply_default(message);
      }
    }
    ssh_message_free(message);
  }

  Log.warningln("SSH: Shell request timeout after %d ms", SSH_SHELL_TIMEOUT_MS);
  return false;
}

void SSHServer::handleSSHSession(void *session) {
  if (!session)
    return;
  specialCharacterMode = false;

  if (!authenticateSession(session)) {
    Log.warningln("SSH: Authentication failed");
    return;
  }

  Log.infoln("SSH: User authenticated successfully");

  ssh_channel channel = nullptr;
  if (!waitForChannelSession(session, (void **)&channel)) {
    Log.errorln("SSH: No channel session requested");
    return;
  }

  Log.infoln("SSH: Channel opened");

  if (!waitForShellRequest(session, channel)) {
    Log.errorln("SSH: No shell requested");
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return;
  }

  Log.infoln("SSH: Shell session started");
  activeSSHSession = true;
  sendWelcomeMessage(channel);

  uint8_t sshToSerialBuffer[128];
  uint8_t serialToSSHBuffer[SSH_QUEUE_ITEM_SIZE];
  uint32_t sessionStartTime = millis();

  while (ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel)) {
    if (millis() - sessionStartTime > 3600000) { // 1 hour timeout
      Log.warningln("SSH: Session timeout after 1 hour");
      ssh_channel_write(
          channel, "\r\nSSH session timeout (1 hour). Disconnecting...\r\n",
          52);
      break;
    }
    int nbytes = ssh_channel_read_nonblocking(channel, sshToSerialBuffer,
                                              sizeof(sshToSerialBuffer), 0);
    if (nbytes > 0 && serialWrite) {
      String specialCharacterResponse =
          handleSpecialCharacter(sshToSerialBuffer[0]);
      specialCharacterResponse.replace("\n", "\r\n");
      if (specialCharacterResponse == "TERMINATE") {
        const char *msg = "See you later alligator!...\r\n";
        ssh_channel_write(channel, msg, strlen(msg));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return;
      }
      if (specialCharacterResponse == "RESET") {
        const char *msg = "Bye bye, cruel world… BRB rebooting!...\r\n";
        ssh_channel_write(channel, msg, strlen(msg));
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
      serialWrite(nonstd::span<const uint8_t>(sshToSerialBuffer,
                                              static_cast<size_t>(nbytes)));
    }

    if (serialToSSHQueue &&
        xQueueReceive(serialToSSHQueue, serialToSSHBuffer,
                      pdMS_TO_TICKS(SSH_QUEUE_TIMEOUT_MS)) == pdTRUE) {
      size_t actualSize = serialToSSHBuffer[0];
      if (actualSize > 0 && actualSize <= SSH_QUEUE_PAYLOAD_SIZE) {
        Log.verboseln("$ttyS1->ssh$: %d bytes", actualSize);
        ssh_channel_write(channel, serialToSSHBuffer + 1, actualSize);
      }
    }
  }

  Log.infoln("SSH: Session ended");
  ssh_channel_close(channel);
  ssh_channel_free(channel);
  activeSSHSession = false;
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
  ssh_key tempHostkey = nullptr;
  rc = ssh_pki_generate(SSH_KEYTYPE_RSA, SSH_RSA_KEY_BITS, &tempHostkey);
  if (rc != SSH_OK || !tempHostkey) {
    Log.errorln("%s: Failed to generate RSA host key", __PRETTY_FUNCTION__);
    ssh_bind_free((ssh_bind)sshBind);
    sshBind = nullptr;
    ssh_finalize();
    return;
  }

  Log.infoln("%s: Setting host key for SSH bind", __PRETTY_FUNCTION__);
  rc = ssh_bind_options_set((ssh_bind)sshBind, SSH_BIND_OPTIONS_IMPORT_KEY,
                            tempHostkey);
  if (rc != SSH_OK) {
    Log.errorln("%s: Failed to set host key", __PRETTY_FUNCTION__);
    ssh_key_free(tempHostkey);
    ssh_bind_free((ssh_bind)sshBind);
    sshBind = nullptr;
    ssh_finalize();
    return;
  }

  // Store hostkey for later cleanup - DO NOT free here as sshBind references it
  hostKey = tempHostkey;
  Log.infoln("SSH Server: RSA host key generated successfully");

  Log.infoln("%s: Starting listening", __PRETTY_FUNCTION__);
  if (ssh_bind_listen((ssh_bind)sshBind) < 0) {
    Log.errorln("SSH Server: Failed to listen on port 22: %s",
                ssh_get_error((ssh_bind)sshBind));
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
              SSH_TASK_STACK_SIZE, // Stack size
              this,
              SSH_TASK_PRIORITY, // Priority
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
    ssh_session newSession = ssh_new();
    if (!newSession) {
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    int rc = ssh_bind_accept((ssh_bind)sshBind, newSession);
    if (rc != SSH_OK) {
      ssh_free(newSession);
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    Log.infoln("SSH Task: New connection accepted");

    if (ssh_handle_key_exchange(newSession) != SSH_OK) {
      Log.errorln("SSH Task: Key exchange failed: %s",
                  ssh_get_error(newSession));
      ssh_disconnect(newSession);
      ssh_free(newSession);
      continue;
    }

    Log.infoln("SSH Task: Key exchange successful");
    handleSSHSession(newSession);
    ssh_disconnect(newSession);
    ssh_free(newSession);
  }

  Log.infoln("SSH Task: Stopped");
  vTaskDelete(nullptr);
}

bool SSHServer::isRunning() const { return running; }

} // namespace jrb::wifi_serial
