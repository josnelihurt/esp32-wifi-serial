#pragma once

#include "interfaces/itask.h"
#include "handlers/button_handler.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

class TriplePressCheckSetupTask final : public ITask {
private:
    ButtonHandler& buttonHandler;
    void (*resetFunc)();

public:
    TriplePressCheckSetupTask(ButtonHandler& handler, void (*reset)())
        : buttonHandler(handler), resetFunc(reset) {}
    
    void setup() override;
    void loop() override {}
};

}  // namespace jrb::wifi_serial

