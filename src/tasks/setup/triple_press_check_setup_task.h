#pragma once

#include "interfaces/itask.h"
#include "interfaces/ibutton_handler.h"
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

class TriplePressCheckSetupTask final : public ITask {
private:
    IButtonHandler& buttonHandler;
    std::function<void()> resetFunc;

public:
    TriplePressCheckSetupTask(IButtonHandler& handler, std::function<void()> reset)
        : buttonHandler(handler), resetFunc(reset) {}
    
    void setup() override;
    void loop() override {}
};

}  // namespace jrb::wifi_serial

