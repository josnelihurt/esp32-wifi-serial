#pragma once

#include "interfaces/itask.h"
#include "infrastructure/hardware/serial_command_handler.h"

namespace jrb::wifi_serial {

class SystemLoopTask final : public ITask {
private:
    SerialCommandHandler* commandHandler;

public:
    explicit SystemLoopTask(SerialCommandHandler* handler)
        : commandHandler(handler) {}
    
    void loop() override {
        if (commandHandler) {
            commandHandler->handle();
        }
    }
};

}  // namespace jrb::wifi_serial

