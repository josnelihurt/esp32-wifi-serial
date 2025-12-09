#pragma once

#include "interfaces/itask.h"
#include "config.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

class PinModeSetupTask final : public ITask {
public:
    void setup() override {
        pinMode(LED_PIN, OUTPUT);
        pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
        digitalWrite(LED_PIN, HIGH);
    }
    void loop() override {}
};

}  // namespace jrb::wifi_serial

