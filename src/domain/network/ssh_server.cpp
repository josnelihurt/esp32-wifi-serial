#include "ssh_server.h"
#include "domain/config/preferences_storage.h"
#include "system_info.h"
#include <ArduinoLog.h>
#include "libssh_esp32.h"
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <WiFi.h>

namespace jrb::wifi_serial {

SSHServer::SSHServer(PreferencesStorage& storage, SystemInfo& sysInfo)
    : preferencesStorage(storage), systemInfo(sysInfo), sshBind(nullptr),
      running(false), sshTaskHandle(nullptr) {
    Log.traceln(__PRETTY_FUNCTION__);
}

SSHServer::~SSHServer() {
    stop();
    if (sshBind) {
        ssh_bind_free((ssh_bind)sshBind);
    }
}

void SSHServer::setSerialCallbacks(SerialWriteCallback writeCallback, SerialReadCallback readCallback) {
    serialWrite = writeCallback;
    serialRead = readCallback;
}

bool SSHServer::authenticateUser(const char* user, const char* password) {
    bool userMatch = preferencesStorage.webUser.equals(user);
    bool passMatch = preferencesStorage.webPassword.equals(password);

    Log.infoln("SSH auth attempt - user: %s, result: %s", user, (userMatch && passMatch) ? "SUCCESS" : "FAILED");
    return userMatch && passMatch;
}

void SSHServer::sendWelcomeMessage(void* channel) {
    if (!channel) return;

    ssh_channel chan = (ssh_channel)channel;

    // Send welcome banner
    const char* banner =
        "\r\n"
        "╔══════════════════════════════════════════════════════════╗\r\n"
        "║                                                          ║\r\n"
        "║          ESP32 WiFi-Serial Bridge - SSH Console         ║\r\n"
        "║                                                          ║\r\n"
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
        "\r\n"
        "Type Ctrl+C to disconnect\r\n"
        "\r\n",
        preferencesStorage.deviceName.c_str(),
        WiFi.localIP().toString().c_str(),
        WiFi.macAddress().c_str(),
        preferencesStorage.mqttBroker.c_str(),
        preferencesStorage.mqttPort,
        preferencesStorage.baudRateTty1
    );

    ssh_channel_write(chan, sysInfoBuf, strlen(sysInfoBuf));
}

void SSHServer::handleSSHSession(void* session) {
    if (!session) return;

    ssh_session sshSession = (ssh_session)session;
    ssh_message message;
    ssh_channel channel = nullptr;
    bool authenticated = false;

    // Authentication loop
    do {
        message = ssh_message_get(sshSession);
        if (!message) break;

        int type = ssh_message_type(message);
        int subtype = ssh_message_subtype(message);

        if (type == SSH_REQUEST_AUTH && subtype == SSH_AUTH_METHOD_PASSWORD) {
            const char* user = ssh_message_auth_user(message);
            const char* pass = ssh_message_auth_password(message);

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
        if (!message) break;

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
        if (!message) break;

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

    // Send welcome message
    sendWelcomeMessage(channel);

    // Main SSH <-> Serial bridge loop
    char buffer[128];
    while (ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel)) {
        // Read from SSH client, write to serial
        int nbytes = ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer), 0);
        if (nbytes > 0 && serialWrite) {
            String str = String(buffer, nbytes);
            Log.noticeln("$ssh$ (write-to-tty1): %s", str.c_str());
            serialWrite(buffer, nbytes);
        }

        // Read from serial, write to SSH client
        if (serialRead) {
            int serialBytes = serialRead(buffer, sizeof(buffer));
            if (serialBytes > 0) {
                String str = String(buffer, serialBytes);
                Log.noticeln("$ssh$ (read-from-tty1): %s", str.c_str());
                ssh_channel_write(channel, buffer, serialBytes);
            }
        }

        // Small delay to prevent tight loop
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    Log.infoln("SSH: Session ended");
    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

void SSHServer::setup() {
    Log.infoln(__PRETTY_FUNCTION__);

    // Check if WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
        Log.warningln("SSH Server: WiFi not connected, skipping setup");
        return;
    }

    // Initialize libssh for ESP32
    libssh_begin();

    // Initialize libssh
    int rc = ssh_init();
    if (rc != SSH_OK) {
        Log.errorln("SSH Server: Failed to initialize libssh: %d", rc);
        return;
    }

    // Create SSH bind (server listener)
    sshBind = ssh_bind_new();
    if (!sshBind) {
        Log.errorln("SSH Server: Failed to create SSH bind");
        ssh_finalize();
        return;
    }

    // Configure SSH bind options
    ssh_bind_options_set((ssh_bind)sshBind, SSH_BIND_OPTIONS_BINDPORT_STR, "22");

    // Generate RSA host key
    ssh_key hostkey = nullptr;
    rc = ssh_pki_generate(SSH_KEYTYPE_RSA, 2048, &hostkey);
    if (rc != SSH_OK || !hostkey) {
        Log.errorln("SSH Server: Failed to generate RSA host key");
        ssh_bind_free((ssh_bind)sshBind);
        sshBind = nullptr;
        ssh_finalize();
        return;
    }

    // Set the host key for the SSH bind
    rc = ssh_bind_options_set((ssh_bind)sshBind, SSH_BIND_OPTIONS_IMPORT_KEY, hostkey);
    if (rc != SSH_OK) {
        Log.errorln("SSH Server: Failed to set host key");
        ssh_key_free(hostkey);
        ssh_bind_free((ssh_bind)sshBind);
        sshBind = nullptr;
        ssh_finalize();
        return;
    }

    Log.infoln("SSH Server: RSA host key generated successfully");

    // Start listening
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

    // Start SSH task
    xTaskCreate(
        sshTask,
        "SSH_Server",
        8192,  // Stack size
        this,
        1,     // Priority
        &sshTaskHandle
    );

    if (sshTaskHandle) {
        Log.infoln("SSH Server: Task created successfully");
    } else {
        Log.errorln("SSH Server: Failed to create task");
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

void SSHServer::sshTask(void* parameter) {
    SSHServer* server = static_cast<SSHServer*>(parameter);
    server->runSSHTask();
}

void SSHServer::runSSHTask() {
    Log.infoln("SSH Task: Started");

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
                Log.errorln("SSH Task: Key exchange failed: %s", ssh_get_error(newSession));
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

bool SSHServer::isRunning() const {
    return running;
}

}  // namespace jrb::wifi_serial
