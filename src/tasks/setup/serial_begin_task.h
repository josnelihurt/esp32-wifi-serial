#pragma once

#include "interfaces/itask.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

class SerialBeginTask final : public ITask {
private:
    int baudRate;

public:
    explicit SerialBeginTask(int baud = 115200) : baudRate(baud) {}
    void setup() override {
        Serial.begin(baudRate);
        delay(1000);
    }
    void loop() override {}
};

}  // namespace jrb::wifi_serial

