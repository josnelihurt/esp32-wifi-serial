#pragma once

namespace jrb::wifi_serial {

/**
 * @brief Interface for serial command processing
 * 
 * Provides abstraction for handling special commands received via serial port,
 * such as info display and debug mode toggling.
 */
class ISerialCommandHandler {
public:
    virtual ~ISerialCommandHandler() = default;
    
    /**
     * @brief Process incoming serial commands
     * 
     * Checks for command prefix and executes corresponding actions.
     * Must be called regularly in the main loop to process commands.
     */
    virtual void handle() = 0;
};

}  // namespace jrb::wifi_serial

