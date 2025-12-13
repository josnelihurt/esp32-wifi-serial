#pragma once

#include <WiFi.h>
#include <Arduino.h>
#include <Preferences.h>

namespace jrb::wifi_serial {

/**
 * @brief Interface for WiFi network management
 * 
 * Provides abstraction for WiFi connection management, including station mode,
 * access point mode, and configuration retrieval.
 */
class IWiFiManager {
public:
    virtual ~IWiFiManager() = default;
    
    /**
     * @brief Attempt to connect to configured WiFi network
     * @return true if connection successful, false otherwise
     */
    virtual bool connect() = 0;
    
    /**
     * @brief Process WiFi events and maintain connection
     * Must be called regularly in the main loop
     */
    virtual void loop() = 0;
    
    /**
     * @brief Check if device is in access point mode
     * @return true if in AP mode, false if in station mode
     */
    virtual bool isAPMode() const = 0;
    
    /**
     * @brief Get access point IP address
     * @return IP address when in AP mode
     */
    virtual IPAddress getAPIP() const = 0;
};

}  // namespace jrb::wifi_serial

