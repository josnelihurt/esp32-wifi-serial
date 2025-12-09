#pragma once

#include "config.h"
#include "interfaces/iserial_bridge.h"
#include <Arduino.h>
#include <HardwareSerial.h>

namespace jrb::wifi_serial {

class SerialLog;
class IMqttClient;

class SerialBridge final : public ISerialBridge {
private:
    HardwareSerial* serial1;
    char buffer0[SERIAL_BUFFER_SIZE];
    char buffer1[SERIAL_BUFFER_SIZE];
    int buffer0Index;
    int buffer1Index;
    
    SerialLog* serial0Log{};
    SerialLog* serial1Log{};
    IMqttClient* mqttClient{};

public:
    SerialBridge();
    ~SerialBridge() = default;

    void begin() override;
    void begin(int baud0, int baud1) override;
    
    void setLogs(SerialLog& log0, SerialLog& log1) override;
    void setMqttHandler(IMqttClient* client) override;
    
    int readSerial0(char* buffer, int maxLen) override;
    int readSerial1(char* buffer, int maxLen) override;
    
    void writeSerial0(const char* data, int length) override;
    void writeSerial1(const char* data, int length) override;
    void writeSerial0(const String& data) override;
    void writeSerial1(const String& data) override;

    bool available0() const override;
    bool available1() const override;
    
    void writeToSerialAndLog(int portIndex, const String& serialData, const String& logData) override;
    void handleSerialToMqttAndWeb(int portIndex, const char* data, unsigned int length) override;
    void handleWebToSerialAndMqtt(int portIndex, const String& data) override;
    void handleMqttToSerialAndWeb(int portIndex, const char* data, unsigned int length) override;
};

}  // namespace jrb::wifi_serial

