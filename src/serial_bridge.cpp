#include "serial_bridge.h"
#include "config.h"
#include "serial_log.h"
#include "mqtt_client.h"

namespace jrb::wifi_serial {

SerialBridge::SerialBridge()
    : serial1{}
    , buffer0Index{0}
    , buffer1Index{0}
{
}


void SerialBridge::begin() {
    begin(SERIAL0_BAUD, SERIAL1_BAUD);
}

void SerialBridge::begin(int baud0, int baud1) {
    Serial.begin(baud0);
    
    if (baud1 > 0) {
        serial1 = new HardwareSerial(1);
        serial1->begin(baud1, SERIAL_8N1, 4, 5);
    }
}

int SerialBridge::readSerial0(char* buffer, int maxLen) {
    if (!available0()) {
        return 0;
    }

    int count = 0;
    while (Serial.available() && count < maxLen - 1) {
        char c = Serial.read();
        if (c != '\0') {
            buffer[count++] = c;
        }
    }
    buffer[count] = '\0';
    return count;
}

int SerialBridge::readSerial1(char* buffer, int maxLen) {
    if (!serial1 || !available1()) {
        return 0;
    }

    int count = 0;
    while (serial1->available() && count < maxLen - 1) {
        char c = serial1->read();
        if (c != '\0') {
            buffer[count++] = c;
        }
    }
    buffer[count] = '\0';
    return count;
}

void SerialBridge::writeSerial0(const char* data, int length) {
    Serial.write((uint8_t*)data, length);
}

void SerialBridge::writeSerial1(const char* data, int length) {
    if (!serial1) return;
    serial1->write((uint8_t*)data, length);
}

void SerialBridge::writeSerial0(const String& data) {
    writeSerial0(data.c_str(), data.length());
}

void SerialBridge::writeSerial1(const String& data) {
    writeSerial1(data.c_str(), data.length());
}

bool SerialBridge::available0() const {
    return Serial.available() > 0;
}

bool SerialBridge::available1() const {
    return serial1 && serial1->available() > 0;
}

void SerialBridge::setLogs(SerialLog& log0, SerialLog& log1) {
    serial0Log = &log0;
    serial1Log = &log1;
}

void SerialBridge::setMqttHandler(MqttClient* client) {
    mqttClient = client;
}

void SerialBridge::writeToSerialAndLog(int portIndex, const String& serialData, const String& logData) {
    if (portIndex == 0) {
        writeSerial0(serialData);
        if (serial0Log) {
            serial0Log->append(logData);
        }
    } else {
        writeSerial1(serialData);
        if (serial1Log) {
            serial1Log->append(logData);
        }
    }
}

void SerialBridge::handleSerialToMqttAndWeb(int portIndex, const char* data, unsigned int length) {
    if (length == 0 || portIndex < 0 || portIndex > 1) return;
    
    SerialLog* log = (portIndex == 0) ? serial0Log : serial1Log;
    if (log) {
        log->append(data, length);
    }
    
    if (mqttClient) {
        mqttClient->appendToBuffer(portIndex, data, length);
    }
}

void SerialBridge::handleWebToSerialAndMqtt(int portIndex, const String& data) {
    String dataWithNewline = data + "\n";
    String webMsg = "$web$ " + dataWithNewline;
    
    writeToSerialAndLog(portIndex, dataWithNewline, webMsg);
    
    if (!mqttClient) {
        Serial.println("MQTT client not found");
        return;
    }

    if (!mqttClient->isConnected()) {
        Serial.println("MQTT client not connected");
        return;
    }

    if (portIndex == 0) {
        mqttClient->publishTty0(webMsg);
    } else {
        mqttClient->publishTty1(webMsg);
    }
}

void SerialBridge::handleMqttToSerialAndWeb(int portIndex, const char* data, unsigned int length) {
    String dataWithNewline = String(data, length) + "\n";
    String mqttMsg = "$mqtt$ " + dataWithNewline;
    
    writeToSerialAndLog(portIndex, dataWithNewline, mqttMsg);
}

}  // namespace jrb::wifi_serial

