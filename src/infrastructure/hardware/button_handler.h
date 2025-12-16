#pragma once

#include "config.h"
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

// Handles button input and detects triple press events.
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

public:
  /**
   * @brief Constructor for the ButtonHandler class.
   *
   * Initializes the button handler with a function to execute on triple press.
   *
   * @param printWelcome The function to be called when a triple press is
   * detected.
   */
  explicit ButtonHandler() = default;
  ~ButtonHandler() = default;

  // Handles button input and detects triple press events.
  bool checkTriplePress();
};

} // namespace jrb::wifi_serial
