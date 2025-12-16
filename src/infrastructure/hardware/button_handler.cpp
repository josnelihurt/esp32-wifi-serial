#include "button_handler.h"
#include "config.h"

namespace jrb::wifi_serial {

bool ButtonHandler::checkTriplePress() {
  bool currentButtonState = digitalRead(BOOT_BUTTON_PIN);
  unsigned long now = millis();

  if (currentButtonState == LOW && lastButtonState == HIGH) {
    if (now - buttonPressTime > BUTTON_DEBOUNCE_MS) {
      buttonPressCount++;
      buttonPressTime = now;

      if (buttonPressCount == 1) {
        digitalWrite(LED_PIN, LOW);
        delay(50);
        digitalWrite(LED_PIN, HIGH);
      } else if (buttonPressCount == 2) {
        digitalWrite(LED_PIN, LOW);
        delay(50);
        digitalWrite(LED_PIN, HIGH);
      } else if (buttonPressCount >= 3) {
        for (int i = 0; i < 5; i++) {
          digitalWrite(LED_PIN, LOW);
          delay(100);
          digitalWrite(LED_PIN, HIGH);
          delay(100);
        }
        lastButtonState = currentButtonState;
        return true;
      }
    }
  }

  if (now - buttonPressTime > TRIPLE_PRESS_TIMEOUT) {
    buttonPressCount = 0;
  }

  lastButtonState = currentButtonState;

  if (currentButtonState == LOW && now - buttonPressTime > 200 &&
      buttonPressCount == 0) {
    buttonPressTime = now;
  }

  return false;
}

} // namespace jrb::wifi_serial
