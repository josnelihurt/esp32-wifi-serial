#pragma once

#include "interfaces/itask.h"
#include "system_info.h"

namespace jrb::wifi_serial {

class PrintWelcomeTask final : public ITask {
private:
    SystemInfo& systemInfo;

public:
    explicit PrintWelcomeTask(SystemInfo& info) : systemInfo(info) {}
    void setup() override {
        systemInfo.printWelcomeMessage();
    }
    void loop() override {}
};

}  // namespace jrb::wifi_serial

