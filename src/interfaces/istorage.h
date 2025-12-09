#pragma once

#include <Arduino.h>

namespace jrb::wifi_serial {

/**
 * @brief Interface for persistent storage operations
 * 
 * Provides abstraction for storing and retrieving device configuration,
 * including WiFi credentials, MQTT settings, and topic names.
 */
class IStorage {
public:
    virtual ~IStorage() = default;
    
    /**
     * @brief Load configuration from persistent storage
     */
    virtual void load() = 0;
    
    /**
     * @brief Save current configuration to persistent storage
     */
    virtual void save() = 0;
    
    /**
     * @brief Clear all stored configuration
     */
    virtual void clear() = 0;
    
    /**
     * @brief Get device name
     * @return Reference to device name string
     */
    virtual const String& deviceName() const = 0;
    
    /**
     * @brief Get MQTT broker address
     * @return Reference to broker address string
     */
    virtual const String& mqttBroker() const = 0;
    
    /**
     * @brief Get MQTT broker port
     * @return Port number
     */
    virtual int mqttPort() const = 0;
    
    /**
     * @brief Get MQTT username
     * @return Reference to username string
     */
    virtual const String& mqttUser() const = 0;
    
    /**
     * @brief Get MQTT password
     * @return Reference to password string
     */
    virtual const String& mqttPassword() const = 0;
    
    /**
     * @brief Get MQTT topic for serial port 0 receive
     * @return Reference to topic string
     */
    virtual const String& topicTty0Rx() const = 0;
    
    /**
     * @brief Get MQTT topic for serial port 0 transmit
     * @return Reference to topic string
     */
    virtual const String& topicTty0Tx() const = 0;
    
    /**
     * @brief Get MQTT topic for serial port 1 receive
     * @return Reference to topic string
     */
    virtual const String& topicTty1Rx() const = 0;
    
    /**
     * @brief Get MQTT topic for serial port 1 transmit
     * @return Reference to topic string
     */
    virtual const String& topicTty1Tx() const = 0;
    
    /**
     * @brief Set device name
     * @param v Device name value
     */
    virtual void deviceName(const String& v) = 0;
    
    /**
     * @brief Set MQTT broker address
     * @param v Broker address value
     */
    virtual void mqttBroker(const String& v) = 0;
    
    /**
     * @brief Set MQTT broker port
     * @param v Port number value
     */
    virtual void mqttPort(int v) = 0;
    
    /**
     * @brief Set MQTT username
     * @param v Username value
     */
    virtual void mqttUser(const String& v) = 0;
    
    /**
     * @brief Set MQTT password
     * @param v Password value
     */
    virtual void mqttPassword(const String& v) = 0;
    
    /**
     * @brief Set MQTT topic for serial port 0 receive
     * @param v Topic string value
     */
    virtual void topicTty0Rx(const String& v) = 0;
    
    /**
     * @brief Set MQTT topic for serial port 0 transmit
     * @param v Topic string value
     */
    virtual void topicTty0Tx(const String& v) = 0;
    
    /**
     * @brief Set MQTT topic for serial port 1 receive
     * @param v Topic string value
     */
    virtual void topicTty1Rx(const String& v) = 0;
    
    /**
     * @brief Set MQTT topic for serial port 1 transmit
     * @param v Topic string value
     */
    virtual void topicTty1Tx(const String& v) = 0;
};

}  // namespace jrb::wifi_serial

