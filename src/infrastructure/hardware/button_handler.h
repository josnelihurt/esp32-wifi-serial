#pragma once

#include "config.h"
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

/**
 * @brief Handles button input and detects triple press events.
 *
 * This class provides functionality to monitor button presses and detect when a triple press occurs,
 * which can be used to trigger specific actions in the firmware.
 */
class ButtonHandler final {
private:
    /**
     * @brief Timestamp of the last button check.
     */
    unsigned long lastButtonCheck{0};

    /**
     * @brief Count of consecutive button presses.
     */
    int buttonPressCount{0};

    /**
     * @brief Timestamp of the last button press.
     */
    unsigned long buttonPressTime{0};

    /**
     * @brief Previous state of the button (HIGH or LOW).
     */
    bool lastButtonState{HIGH};

    /**
     * @brief Function to execute when a triple press is detected.
     */
    std::function<void()> printWelcomeFunc;

public:
    /**
     * @brief Constructor for the ButtonHandler class.
     *
     * Initializes the button handler with a function to execute on triple press.
     *
     * @param printWelcome The function to be called when a triple press is detected.
     */
    explicit ButtonHandler(std::function<void()> printWelcome) : printWelcomeFunc(printWelcome) {}
    
    /**
     * @brief Checks if a triple press event has occurred.
     *
     * This method evaluates the button state and determines if three consecutive presses
     * have been detected within a predefined time interval. If so, it triggers the associated function.
     *
     * @return True if a triple press is detected, false otherwise.
     */
    bool checkTriplePress();
};

}  // namespace jrb::wifi_serial
