#pragma once

#include <Arduino.h>

namespace jrb::wifi_serial {

/**
 * @brief Interface for serial port logging
 * 
 * Provides abstraction for circular buffer logging of serial port data,
 * allowing retrieval of new data since last read position.
 */
class ISerialLog {
public:
    virtual ~ISerialLog() = default;
    
    /**
     * @brief Append data to log buffer
     * @param data Data buffer to append
     * @param length Length of data buffer
     */
    virtual void append(const char* data, int length) = 0;
    
    /**
     * @brief Append string data to log buffer
     * @param data String data to append
     */
    virtual void append(const String& data) = 0;
    
    /**
     * @brief Get new data since last read position
     * @param lastReadPos Reference to last read position (updated on return)
     * @return String containing new data
     */
    virtual String getNewData(int& lastReadPos) = 0;
    
    /**
     * @brief Reset log buffer and read position
     */
    virtual void reset() = 0;
    
    /**
     * @brief Check if log buffer contains new data
     * @return true if new data available, false otherwise
     */
    virtual bool hasData() const = 0;
};

}  // namespace jrb::wifi_serial

