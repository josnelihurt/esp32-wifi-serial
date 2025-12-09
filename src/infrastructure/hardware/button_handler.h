#pragma once

#include "config.h"
#include "interfaces/ibutton_handler.h"
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

class ButtonHandler final : public IButtonHandler {
private:
    unsigned long lastButtonCheck{0};
    int buttonPressCount{0};
    unsigned long buttonPressTime{0};
    bool lastButtonState{HIGH};
    std::function<void()> printWelcomeFunc;

public:
    explicit ButtonHandler(std::function<void()> printWelcome) : printWelcomeFunc(printWelcome) {}
    
    bool checkTriplePress() override;
};

}  // namespace jrb::wifi_serial

