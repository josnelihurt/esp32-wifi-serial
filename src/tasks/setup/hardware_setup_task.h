#pragma once

#include "interfaces/itask.h"
#include "config.h"
#include "serial_bridge.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

class HardwareSetupTask final : public ITask {
private:
    SerialBridge& serialBridge;
    int serialBaudRate;

public:
    explicit HardwareSetupTask(SerialBridge& bridge, int baud = SERIAL0_BAUD)
        : serialBridge(bridge), serialBaudRate(baud) {}
    
    void setup() override {
        pinMode(LED_PIN, OUTPUT);
        pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
        digitalWrite(LED_PIN, HIGH);
        
        Serial.begin(serialBaudRate);
        delay(1000);
        
        serialBridge.begin();
    }
    
    void loop() override {}
};

}  // namespace jrb::wifi_serial

