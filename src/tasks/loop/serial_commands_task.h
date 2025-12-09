#pragma once

#include "interfaces/itask.h"
#include "handlers/serial_command_handler.h"

namespace jrb::wifi_serial {

class SerialCommandsTask final : public ITask {
private:
    SerialCommandHandler* handler;

public:
    explicit SerialCommandsTask(SerialCommandHandler* h) : handler(h) {}
    void loop() override { if (handler) handler->handle(); }
};

}  // namespace jrb::wifi_serial

