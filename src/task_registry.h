#pragma once

#include "interfaces/itask.h"
#include <vector>

namespace jrb::wifi_serial {

class TaskRegistry final {
private:
    std::vector<ITask*> tasks;
    bool setupCalled{false};

public:
    TaskRegistry() = default;
    ~TaskRegistry() = default;

    void registerTask(ITask* task);
    void setupAll();
    void loopAll();
    void clear();
};

}  // namespace jrb::wifi_serial

