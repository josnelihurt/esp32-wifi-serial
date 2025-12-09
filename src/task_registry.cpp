#include "task_registry.h"

namespace jrb::wifi_serial {

void TaskRegistry::registerTask(ITask* task) {
    if (!task) return;
    tasks.push_back(task);
}

void TaskRegistry::setupAll() {
    if (setupCalled) return;
    
    for (auto* task : tasks) {
        if (!task) continue;
        task->setup();
    }
    setupCalled = true;
}

void TaskRegistry::loopAll() {
    for (auto* task : tasks) {
        if (!task) continue;
        task->loop();
    }
}

void TaskRegistry::clear() {
    tasks.clear();
    setupCalled = false;
}

}  // namespace jrb::wifi_serial

