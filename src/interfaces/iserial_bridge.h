#pragma once

#include <Arduino.h>

namespace jrb::wifi_serial {

class SerialLog;
class IMqttClient;

/**
 * @brief Interface for serial bridge operations
 * 
 * Provides abstraction for bridging serial port data between hardware serial ports,
 * MQTT, and web interfaces. Handles bidirectional communication for two serial ports.
 */
class ISerialBridge {
public:
    virtual ~ISerialBridge() = default;
    
    /**
     * @brief Initialize serial ports with custom baud rates
     * @param ttyS1BaudRate Baud rate for serial port 1 (0 to disable)
     */
    virtual void begin(int ttyS1BaudRate) = 0;
    
    /**
     * @brief Set log handlers for both serial ports
     * @param log0 Log handler for serial port 0
     * @param log1 Log handler for serial port 1
     */
    virtual void setLogs(SerialLog& log0, SerialLog& log1) = 0;
    
    /**
     * @brief Set MQTT client handler for publishing serial data
     * @param client Reference to MQTT client instance
     */
    virtual void setMqttHandler(IMqttClient& client) = 0;
    
    /**
     * @brief Read available data from serial port 0
     * @param buffer Buffer to store read data
     * @param maxLen Maximum length to read
     * @return Number of bytes read, 0 if no data available
     */
    virtual int readSerial0(char* buffer, int maxLen) = 0;
    
    /**
     * @brief Read available data from serial port 1
     * @param buffer Buffer to store read data
     * @param maxLen Maximum length to read
     * @return Number of bytes read, 0 if no data available
     */
    virtual int readSerial1(char* buffer, int maxLen) = 0;
    
    /**
     * @brief Write data to serial port 0
     * @param data Data buffer to write
     * @param length Length of data buffer
     */
    virtual void writeSerial0(const char* data, int length) = 0;
    
    /**
     * @brief Write data to serial port 1
     * @param data Data buffer to write
     * @param length Length of data buffer
     */
    virtual void writeSerial1(const char* data, int length) = 0;
    
    /**
     * @brief Write string data to serial port 0
     * @param data String data to write
     */
    virtual void writeSerial0(const String& data) = 0;
    
    /**
     * @brief Write string data to serial port 1
     * @param data String data to write
     */
    virtual void writeSerial1(const String& data) = 0;
    
    /**
     * @brief Check if data is available on serial port 0
     * @return true if data available, false otherwise
     */
    virtual bool available0() const = 0;
    
    /**
     * @brief Check if data is available on serial port 1
     * @return true if data available, false otherwise
     */
    virtual bool available1() const = 0;
    
    /**
     * @brief Write data to serial port and log simultaneously
     * @param portIndex Port index (0 or 1)
     * @param serialData Data to write to serial port
     * @param logData Data to write to log
     */
    virtual void writeToSerialAndLog(int portIndex, const String& serialData, const String& logData) = 0;
    
    /**
     * @brief Handle data from serial port, forward to MQTT and web
     * @param portIndex Port index (0 or 1)
     * @param data Data received from serial port
     * @param length Length of data
     */
    virtual void handleSerialToMqttAndWeb(int portIndex, const char* data, unsigned int length) = 0;
    
    /**
     * @brief Handle data from web interface, forward to serial and MQTT
     * @param portIndex Port index (0 or 1)
     * @param data Data received from web interface
     */
    virtual void handleWebToSerialAndMqtt(int portIndex, const String& data) = 0;
    
    /**
     * @brief Handle data from MQTT, forward to serial and web
     * @param portIndex Port index (0 or 1)
     * @param data Data received from MQTT
     * @param length Length of data
     */
    virtual void handleMqttToSerialAndWeb(int portIndex, const char* data, unsigned int length) = 0;
};

}  // namespace jrb::wifi_serial

