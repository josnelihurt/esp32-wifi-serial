#pragma once

#include "interfaces/itask.h"
#include "config.h"
#include "domain/serial/serial_bridge.h"
#include "domain/serial/serial_log.h"
#include "interfaces/imqtt_client.h"
#include <Arduino.h>
#include <ArduinoLog.h>

namespace jrb::wifi_serial {

template<int PortIndex>
class SerialBridgeTask final : public ITask {
private:
    SerialBridge& serialBridge;
    SystemInfo& systemInfo;
    SerialLog& serialLog;
    char* serialBuffer;
    IMqttClient& mqttClient;
    bool& debugEnabled;
    bool cmdPrefixReceived{false};  // State for command detection

    bool available() const {
        if (PortIndex == 0) {
            return serialBridge.available0();
        } else {
            return serialBridge.available1();
        }
    }

    int readSerial(char* buffer, int maxLen) {
        if (PortIndex == 0) {
            return serialBridge.readSerial0(buffer, maxLen);
        } else {
            return serialBridge.readSerial1(buffer, maxLen);
        }
    }

public:
    static constexpr const char* getDebugPrefix() {
        return PortIndex == 0 ? "[DEBUG TTY0] " : "[DEBUG TTY1] ";
    }
    
    SerialBridgeTask(SerialBridge& bridge, SerialLog& log, char* buffer, 
                       IMqttClient& client, bool& debug, SystemInfo& info)
        : serialBridge(bridge), serialLog(log), serialBuffer(buffer), 
          mqttClient(client), debugEnabled(debug), systemInfo(info) {}
    
    void loop() override {
        if (!available()) return;

        int len = readSerial(serialBuffer, SERIAL_BUFFER_SIZE);
        if (len <= 0) return;
        
        if (debugEnabled) {
            Log.traceln("%s%.*s", getDebugPrefix(), len, serialBuffer);
        }
        
        // Check for command sequences (only on ttyS0)
        if (PortIndex == 0) {
            // Process each character in the buffer
            for (int i = 0; i < len; i++) {
                char c = serialBuffer[i];
                
                if (cmdPrefixReceived) {
                    // We received Ctrl+Y previously, now check the command
                    cmdPrefixReceived = false;
                    
                    // Debug: print what character we got after Ctrl+Y
                    if (c >= 32 && c <= 126) {
                        Log.traceln("DEBUG: After Ctrl+Y, got char: 0x%X ('%c')", (int)c, c);
                    } else {
                        Log.traceln("DEBUG: After Ctrl+Y, got char: 0x%X", (int)c);
                    }
                    
                    if (c == CMD_INFO || c == 'I') {
                        systemInfo.logSystemInformation();
                        // Remove this command character from buffer
                        for (int j = i; j < len - 1; j++) {
                            serialBuffer[j] = serialBuffer[j + 1];
                        }
                        len -= 1;
                        i--; // Adjust index since we removed a character
                        continue;
                    } else if (c == CMD_DEBUG || c == 'D') {
                        Log.infoln("Command detected: Ctrl+Y + d (Toggle Debug)");
                        debugEnabled = !debugEnabled;
                        Log.infoln("Debug %s", debugEnabled ? "enabled" : "disabled");
                        // Remove this command character from buffer
                        for (int j = i; j < len - 1; j++) {
                            serialBuffer[j] = serialBuffer[j + 1];
                        }
                        len -= 1;
                        i--; // Adjust index since we removed a character
                        continue;
                    } else {
                        // Not a valid command after Ctrl+Y, keep the Ctrl+Y in buffer
                        // We need to insert Ctrl+Y back at position i-1
                        // But we already passed it, so we need special handling
                        // For now, just let it through
                    }
                }
                
                // Check for new command prefix
                if (c == CMD_PREFIX) {
                    Log.traceln("DEBUG: Ctrl/Y (command prefix) detected");
                    cmdPrefixReceived = true;
                    // Remove Ctrl+Y from buffer since we're handling it as a command
                    for (int j = i; j < len - 1; j++) {
                        serialBuffer[j] = serialBuffer[j + 1];
                    }
                    len -= 1;
                    i--; // Adjust index since we removed a character
                    continue;
                }
            }
        }
        
        if (len > 0) {
            serialBridge.handleSerialToMqttAndWeb(PortIndex, serialBuffer, len);
        }
    }
};

using SerialBridge0Task = SerialBridgeTask<0>;
using SerialBridge1Task = SerialBridgeTask<1>;

}  // namespace jrb::wifi_serial

