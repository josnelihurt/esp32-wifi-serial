#pragma once

#include <Arduino.h>

namespace jrb::wifi_serial {

/**
 * @brief Interface for MQTT client operations
 * 
 * Provides abstraction for MQTT communication, including connection management,
 * topic publishing, and message buffering for serial port data.
 */
class IMqttClient {
public:
    virtual ~IMqttClient() = default;
    
    /**
     * @brief Set the device name used for MQTT client identification
     * @param name Device name string
     */
    virtual void setDeviceName(const String& name) = 0;
    
    /**
     * @brief Configure MQTT topics for serial port communication
     * @param tty0Rx Topic for receiving data on serial port 0
     * @param tty0Tx Topic for transmitting data on serial port 0
     * @param tty1Rx Topic for receiving data on serial port 1
     * @param tty1Tx Topic for transmitting data on serial port 1
     */
    virtual void setTopics(const String& tty0Rx, const String& tty0Tx, 
                          const String& tty1Rx, const String& tty1Tx) = 0;
    
    /**
     * @brief Set callback functions for received MQTT messages
     * @param tty0 Callback function for messages on serial port 0 topic
     * @param tty1 Callback function for messages on serial port 1 topic
     */
    virtual void setCallbacks(void (*tty0)(const char*, unsigned int),
                             void (*tty1)(const char*, unsigned int)) = 0;
    
    /**
     * @brief Connect to MQTT broker
     * @param broker Broker hostname or IP address
     * @param port Broker port number
     * @param user Optional username for authentication
     * @param password Optional password for authentication
     * @return true if connection successful, false otherwise
     */
    virtual bool connect(const char* broker, int port, const char* user = nullptr, 
                        const char* password = nullptr) = 0;
    
    /**
     * @brief Disconnect from MQTT broker
     */
    virtual void disconnect() = 0;
    
    /**
     * @brief Attempt to reconnect to MQTT broker
     * @return true if reconnected, false otherwise
     */
    virtual bool reconnect() = 0;
    
    /**
     * @brief Process MQTT messages and maintain connection
     * Must be called regularly in the main loop
     */
    virtual void loop() = 0;
    
    /**
     * @brief Publish data to serial port 0 topic
     * @param data Data buffer to publish
     * @param length Length of data buffer
     * @return true if publish successful, false otherwise
     */
    virtual bool publishTty0(const char* data, unsigned int length) = 0;
    
    /**
     * @brief Publish data to serial port 1 topic
     * @param data Data buffer to publish
     * @param length Length of data buffer
     * @return true if publish successful, false otherwise
     */
    virtual bool publishTty1(const char* data, unsigned int length) = 0;
    
    /**
     * @brief Publish string data to serial port 0 topic
     * @param data String data to publish
     * @return true if publish successful, false otherwise
     */
    virtual bool publishTty0(const String& data) = 0;
    
    /**
     * @brief Publish string data to serial port 1 topic
     * @param data String data to publish
     * @return true if publish successful, false otherwise
     */
    virtual bool publishTty1(const String& data) = 0;
    
    /**
     * @brief Publish informational message to info topic
     * @param data Information string to publish
     * @return true if publish successful, false otherwise
     */
    virtual bool publishInfo(const String& data) = 0;
    
    /**
     * @brief Check if client is connected to broker
     * @return true if connected, false otherwise
     */
    virtual bool isConnected() const = 0;
    
    /**
     * @brief Manually set connection state
     * @param state Connection state to set
     */
    virtual void setConnected(bool state) = 0;
    
    /**
     * @brief Append data to publish buffer for specified port
     * @param portIndex Port index (0 or 1)
     * @param data Data to append
     * @param length Length of data
     */
    virtual void appendToBuffer(int portIndex, const char* data, unsigned int length) = 0;
    
    /**
     * @brief Flush buffered data for specified port
     * @param portIndex Port index (0 or 1)
     */
    virtual void flushBuffer(int portIndex) = 0;
    
    /**
     * @brief Check if buffer should be flushed based on time interval
     * @param portIndex Port index (0 or 1)
     * @return true if buffer should be flushed, false otherwise
     */
    virtual bool shouldFlushBuffer(int portIndex) const = 0;
};

}  // namespace jrb::wifi_serial

