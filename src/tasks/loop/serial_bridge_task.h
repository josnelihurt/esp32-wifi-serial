#pragma once

#include "interfaces/itask.h"
#include "config.h"
#include "domain/serial/serial_bridge.h"
#include "domain/serial/serial_log.h"
#include "interfaces/imqtt_client.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

template<int PortIndex>
class SerialBridgeTask final : public ITask {
private:
    SerialBridge& serialBridge;
    SerialLog& serialLog;
    char* serialBuffer;
    IMqttClient* mqttClient;
    bool& debugEnabled;

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
                       IMqttClient* client, bool& debug)
        : serialBridge(bridge), serialLog(log), serialBuffer(buffer), 
          mqttClient(client), debugEnabled(debug) {}
    
    void loop() override {
        if (!available()) return;
        
        int len = readSerial(serialBuffer, SERIAL_BUFFER_SIZE);
        if (len <= 0) return;
        
        if (debugEnabled) {
            Serial.print(getDebugPrefix());
            Serial.write((uint8_t*)serialBuffer, len);
            Serial.println();
        }
        
        serialBridge.handleSerialToMqttAndWeb(PortIndex, serialBuffer, len);
    }
};

using SerialBridge0Task = SerialBridgeTask<0>;
using SerialBridge1Task = SerialBridgeTask<1>;

}  // namespace jrb::wifi_serial

