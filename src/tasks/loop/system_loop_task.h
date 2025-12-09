#pragma once

#include "interfaces/itask.h"
#include "interfaces/iserial_command_handler.h"

namespace jrb::wifi_serial {

class SystemLoopTask final : public ITask {
private:
    ISerialCommandHandler* commandHandler;

public:
    explicit SystemLoopTask(ISerialCommandHandler* handler)
        : commandHandler(handler) {}
    
    void loop() override {
        if (commandHandler) {
            commandHandler->handle();
        }
    }
};

}  // namespace jrb::wifi_serial

