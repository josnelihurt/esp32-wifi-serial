#pragma once

#include "interfaces/itask.h"
#include "handlers/button_handler.h"
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

class TriplePressCheckTask final : public ITask {
private:
    ButtonHandler* buttonHandler;
    std::function<void()> resetFunc;

public:
    TriplePressCheckTask(ButtonHandler* handler, std::function<void()> reset) 
        : buttonHandler(handler), resetFunc(reset) {}
    
    void loop() override;
};

}  // namespace jrb::wifi_serial

