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
     * @brief Initialize WiFi manager with preferences storage
     * @param prefs Pointer to Preferences instance for configuration storage
     */
    virtual void begin(::Preferences* prefs) = 0;
    
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
     * @brief Reset all WiFi and configuration settings
     */
    virtual void resetSettings() = 0;
    
    /**
     * @brief Get configured WiFi SSID
     * @return SSID string
     */
    virtual String getSSID() const = 0;
    
    /**
     * @brief Get configured WiFi password
     * @return Password string
     */
    virtual String getPassword() const = 0;
    
    /**
     * @brief Get device name
     * @return Device name string
     */
    virtual String getDeviceName() const = 0;
    
    /**
     * @brief Get MQTT broker address
     * @return Broker address string
     */
    virtual String getMqttBroker() const = 0;
    
    /**
     * @brief Get MQTT broker port
     * @return Port number
     */
    virtual int getMqttPort() const = 0;
    
    /**
     * @brief Get MQTT username
     * @return Username string
     */
    virtual String getMqttUser() const = 0;
    
    /**
     * @brief Get MQTT password
     * @return Password string
     */
    virtual String getMqttPassword() const = 0;
    
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

