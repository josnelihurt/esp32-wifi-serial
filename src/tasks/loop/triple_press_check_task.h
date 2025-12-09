#pragma once

#include "interfaces/itask.h"
#include "handlers/button_handler.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

class TriplePressCheckTask final : public ITask {
private:
    ButtonHandler* buttonHandler;
    void (*resetFunc)();

public:
    TriplePressCheckTask(ButtonHandler* handler, void (*reset)()) 
        : buttonHandler(handler), resetFunc(reset) {}
    
    void loop() override;
};

}  // namespace jrb::wifi_serial

