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
};

}  // namespace jrb::wifi_serial

