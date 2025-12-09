#pragma once

namespace jrb::wifi_serial {

/**
 * @brief Interface for Over-The-Air (OTA) update management
 * 
 * Provides abstraction for OTA firmware update functionality,
 * allowing remote firmware updates via WiFi.
 */
class IOTAManager {
public:
    virtual ~IOTAManager() = default;
    
    /**
     * @brief Initialize and configure OTA update system
     * Sets up OTA handlers, hostname, and password if configured
     */
    virtual void setup() = 0;
};

}  // namespace jrb::wifi_serial

