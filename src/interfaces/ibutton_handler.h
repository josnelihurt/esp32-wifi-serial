#pragma once

namespace jrb::wifi_serial {

/**
 * @brief Interface for button input handling
 * 
 * Provides abstraction for detecting button press patterns,
 * specifically triple-press detection for configuration reset.
 */
class IButtonHandler {
public:
    virtual ~IButtonHandler() = default;
    
    /**
     * @brief Check if triple press pattern was detected
     * 
     * Detects three rapid button presses within timeout window.
     * Used for triggering configuration reset.
     * 
     * @return true if triple press detected, false otherwise
     */
    virtual bool checkTriplePress() = 0;
};

}  // namespace jrb::wifi_serial

