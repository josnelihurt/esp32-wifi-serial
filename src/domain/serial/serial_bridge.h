#pragma once

#include "config.h"
#include "interfaces/iserial_bridge.h"
#include <Arduino.h>
#include <HardwareSerial.h>

namespace jrb::wifi_serial {

class SerialLog;
class IMqttClient;

/**
 * @brief Manages bidirectional communication between serial ports, logs, and MQTT.
 *
 * This class handles data transfer between two serial interfaces (e.g., UART0/UART1), 
 * logs data to a `SerialLog` object, and communicates with an MQTT client for remote monitoring.
 */
class SerialBridge final : public ISerialBridge {
public:
    SerialBridge();
    ~SerialBridge() = default;

    /**
     * @brief Initializes serial communication with specified baud rates for each port.
     *
     * @param ttyS1BaudRate Baud rate for the second serial interface.
     */
    void begin(int ttyS1BaudRate) override;
    
    /**
     * @brief Assigns log buffers for data from each serial port.
     *
     * @param log0 Reference to the log buffer for the first serial interface.
     * @param log1 Reference to the log buffer for the second serial interface.
     */
    void setLogs(SerialLog& log0, SerialLog& log1) override;

    /**
     * @brief Sets the MQTT client used by the bridge for publishing and subscribing.
     *
     * @param client Reference to an `IMqttClient` implementation.
     */
    void setMqttHandler(IMqttClient& client) override;
    
    /**
     * @brief Reads data from the first serial interface into a provided buffer.
     *
     * @param buffer Destination buffer for received data.
     * @param maxLen Maximum number of bytes to read.
     * @return Number of bytes successfully read.
     */
    int readSerial0(char* buffer, int maxLen) override;
    
    /**
     * @brief Reads data from the second serial interface into a provided buffer.
     *
     * @param buffer Destination buffer for received data.
     * @param maxLen Maximum number of bytes to read.
     * @return Number of bytes successfully read.
     */
    int readSerial1(char* buffer, int max_len) override;

    /**
     * @brief Writes data to the first serial interface and logs it.
     *
     * @param data Pointer to a character array containing the data to send.
     * @param length Number of bytes in the data buffer.
     */
    void writeSerial0(const char* data, int length) override;

    /**
     * @brief Writes data to the second serial interface and logs it.
     *
     * @param data Pointer to a character array containing the data to send.
     * @param length Number of bytes in the data buffer.
     */
    void writeSerial1(const char* data, int length) override;

    /**
     * @brief Writes a String object to the first serial interface and logs it.
     *
     * @param data String object containing the data to send.
     */
    void writeSerial0(const String& data) override;

    /**
     * @brief Writes a String object to the second serial interface and logs it.
     *
     * @param data String object containing the data to send.
     */
    void writeSerial1(const String& data) override;

    /**
     * @brief Checks if data is available on the first serial interface.
     *
     * @return True if data is available, false otherwise.
     */
    bool available0() const override;

    /**
     * @brief Checks if data is available on the second serial interface.
     *
     * @return True if data is available, false otherwise.
     */
    bool available1() const override;
    
    /**
     * @brief Sends data to a specified serial port and appends it to the corresponding log buffer.
     *
     * @param portIndex Index of the target serial port (0 or 1).
     * @param serialData Data to send over the serial interface.
     * @param logData Data to append to the log buffer.
     */
    void writeToSerialAndLog(int portIndex, const String& serialData, const String& logData) override;

    /**
     * @brief Handles data received from a serial interface and forwards it via MQTT and web.
     *
     * @param portIndex Index of the source serial port (0 or 1).
     * @param data Pointer to the incoming data buffer.
     * @param length Number of bytes in the data buffer.
     */
    void handleSerialToMqttAndWeb(int portIndex, const char* data, unsigned int length) override;

    /**
     * @brief Handles data received from a web interface and forwards it to serial and MQTT.
     *
     * @param portIndex Index of the target serial port (0 or 1).
     * @param data String object containing the data from the web interface.
     */
    void handleWebToSerialAndMqtt(int portIndex, const String& data) override;

     /**
      * @brief Handles an MQTT message and forwards it to a serial port and web interface.
      *
      * @param portIndex Index of the target serial port (0 or 1).
      * @param data Pointer to the incoming MQTT payload buffer.
      * @param length Number of bytes in the MQTT payload buffer.
      */
    void handleMqttToSerialAndWeb(int portIndex, const char* data, unsigned int length) override;

private:
    HardwareSerial* serial1;
    char buffer0[SERIAL_BUFFER_SIZE];
    char buffer1[SERIAL_BUFFER_SIZE];
    int buffer0Index;
    int buffer1Index;
    
    SerialLog* serial0Log{};
    SerialLog* serial1Log{};
    IMqttClient* mqttClient{};
    
    /**
     * @brief Converts special control characters to visible representations.
     *
     * @param data Input string that may contain control characters.
     * @return String with control characters replaced by visible representations.
     */
    String makeSpecialCharsVisible(const String& data);
};

}  // namespace jrb::wifi_serial
