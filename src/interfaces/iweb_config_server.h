#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include <functional>

namespace jrb::wifi_serial {

/**
 * @brief Interface for web-based configuration server
 * 
 * Provides abstraction for HTTP server that serves a configuration web page,
 * allowing users to configure WiFi and MQTT settings via browser.
 */
class IWebConfigServer {
public:
    /**
     * @brief Callback type for sending data to serial ports from web interface
     * @param portIndex Serial port index (0 or 1)
     * @param data Data string to send
     */
    using SerialSendCallback = std::function<void(int portIndex, const String& data)>;
    
    virtual ~IWebConfigServer() = default;
    
    /**
     * @brief Start the web configuration server
     */
    virtual void setup() = 0;
    
    /**
     * @brief Process HTTP requests and maintain server
     * Must be called regularly in the main loop
     */
    virtual void loop() = 0;
    
    /**
     * @brief Set WiFi and MQTT configuration to display/edit
     * @param ssid WiFi SSID
     * @param password WiFi password
     * @param deviceName Device name
     * @param mqttBroker MQTT broker address
     * @param mqttPort MQTT broker port
     * @param mqttUser MQTT username
     * @param mqttPassword MQTT password
     */
    virtual void setWiFiConfig(const String& ssid, const String& password, const String& deviceName,
                              const String& mqttBroker, int mqttPort, const String& mqttUser,
                              const String& mqttPassword) = 0;
    
    /**
     * @brief Set access point mode state
     * @param apMode true if in AP mode, false if in station mode
     */
    virtual void setAPMode(bool apMode) = 0;
    
    /**
     * @brief Set access point IP address
     * @param ip IP address of the access point
     */
    virtual void setAPIP(const IPAddress& ip) = 0;
};

}  // namespace jrb::wifi_serial

