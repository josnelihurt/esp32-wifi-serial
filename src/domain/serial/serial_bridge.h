#pragma once

#include "config.h"
#include <Arduino.h>
#include <HardwareSerial.h>

namespace jrb::wifi_serial {

class SerialLog;
class MqttClient;

/**
 * @brief Manages bidirectional communication between serial ports, logs, and MQTT.
 *
 * This class handles data transfer between two serial interfaces (e.g., UART0/UART1),
 * logs data to a `SerialLog` object, and communicates with an MQTT client for remote monitoring.
 */
class SerialBridge final {
public:
    SerialBridge(MqttClient& mqttClient);
    ~SerialBridge() = default;

    /**
     * @brief Initializes serial communication with specified baud rates for each port.
     *
     * @param ttyS1BaudRate Baud rate for the second serial interface.
     */
    void setup(int ttyS1BaudRate) ;
    
    /**
     * @brief Assigns log buffers for data from each serial port.
     *
     * @param log0 Reference to the log buffer for the first serial interface.
     * @param log1 Reference to the log buffer for the second serial interface.
     */
    void setLogs(SerialLog& log0, SerialLog& log1) ;

    
    /**
     * @brief Reads data from the first serial interface into a provided buffer.
     *
     * @param buffer Destination buffer for received data.
     * @param maxLen Maximum number of bytes to read.
     * @return Number of bytes successfully read.
     */
    int readSerial0(char* buffer, int maxLen) ;
    
    /**
     * @brief Reads data from the second serial interface into a provided buffer.
     *
     * @param buffer Destination buffer for received data.
     * @param maxLen Maximum number of bytes to read.
     * @return Number of bytes successfully read.
     */
    int readSerial1(char* buffer, int max_len) ;

    /**
     * @brief Writes data to the second serial interface and logs it.
     *
     * @param data Pointer to a character array containing the data to send.
     * @param length Number of bytes in the data buffer.
     */
    void writeSerial0(const char* data, int length) ;

    /**
     * @brief Writes data to the second serial interface and logs it.
     *
     * @param data String object containing the data to send.
     */
    void writeSerial1(const char* data, int length) ;

    /**
     * @brief Checks if data is available on the first serial interface.
     *
     * @return True if data is available, false otherwise.
     */
    bool available0() const ;

    /**
     * @brief Checks if data is available on the second serial interface.
     *
     * @return True if data is available, false otherwise.
     */
    bool available1() const ;

    /**
     * @brief Get pointer to the second serial interface.
     *
     * @return Pointer to serial1 HardwareSerial object, or nullptr if not initialized.
     */
    HardwareSerial* getSerial1() { return serial1; }
    
    /**
     * @brief Sends data to a specified serial port and appends it to the corresponding log buffer.
     *
     * @param portIndex Index of the target serial port (0 or 1).
     * @param serialData Data to send over the serial interface.
     * @param logData Data to append to the log buffer.
     */
    void writeToSerialAndLog(int portIndex, const String& serialData, const String& logData) ;


     /**
      * @brief Handles an MQTT message and forwards it to a serial port and web interface.
      *
      * @param portIndex Index of the target serial port (0 or 1).
      * @param data Pointer to the incoming MQTT payload buffer.
      * @param length Number of bytes in the MQTT payload buffer.
      */
    void handleMqttToSerialAndWeb(int portIndex, const char* data, unsigned int length) ;

private:
    HardwareSerial* serial1;

    SerialLog* serial0Log{};
    SerialLog* serial1Log{};
    MqttClient& mqttClient;
    
    /**
     * @brief Converts special control characters to visible representations.
     *
     * @param data Input string that may contain control characters.
     * @return String with control characters replaced by visible representations.
     */
    String makeSpecialCharsVisible(const String& data);
};

}  // namespace jrb::wifi_serial
