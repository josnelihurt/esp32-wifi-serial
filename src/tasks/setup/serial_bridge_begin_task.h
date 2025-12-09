#pragma once

#include "interfaces/itask.h"
#include "serial_bridge.h"

namespace jrb::wifi_serial {

class SerialBridgeBeginTask final : public ITask {
private:
    SerialBridge& serialBridge;

public:
    explicit SerialBridgeBeginTask(SerialBridge& bridge) : serialBridge(bridge) {}
    void setup() override {
        serialBridge.begin();
    }
    void loop() override {}
};

}  // namespace jrb::wifi_serial

