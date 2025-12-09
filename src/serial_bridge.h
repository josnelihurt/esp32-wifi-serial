#pragma once

#include "config.h"
#include <Arduino.h>
#include <HardwareSerial.h>

namespace jrb::wifi_serial {

class SerialLog;
class MqttClient;

class SerialBridge final {
private:
    HardwareSerial* serial1;
    char buffer0[SERIAL_BUFFER_SIZE];
    char buffer1[SERIAL_BUFFER_SIZE];
    int buffer0Index;
    int buffer1Index;
    
    SerialLog* serial0Log{};
    SerialLog* serial1Log{};
    MqttClient* mqttClient{};

public:
    SerialBridge();
    ~SerialBridge() = default;

    void begin();
    void begin(int baud0, int baud1);
    
    void setLogs(SerialLog& log0, SerialLog& log1);
    void setMqttHandler(MqttClient* client);
    
    int readSerial0(char* buffer, int maxLen);
    int readSerial1(char* buffer, int maxLen);
    
    void writeSerial0(const char* data, int length);
    void writeSerial1(const char* data, int length);
    void writeSerial0(const String& data);
    void writeSerial1(const String& data);

    bool available0() const;
    bool available1() const;
    
    void writeToSerialAndLog(int portIndex, const String& serialData, const String& logData);
    void handleSerialToMqttAndWeb(int portIndex, const char* data, unsigned int length);
    void handleWebToSerialAndMqtt(int portIndex, const String& data);
    void handleMqttToSerialAndWeb(int portIndex, const char* data, unsigned int length);
};

}  // namespace jrb::wifi_serial

