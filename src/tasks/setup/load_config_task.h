#pragma once

#include "interfaces/itask.h"
#include "preferences_storage.h"

namespace jrb::wifi_serial {

class LoadConfigTask final : public ITask {
private:
    PreferencesStorage& configManager;

public:
    explicit LoadConfigTask(PreferencesStorage& config) : configManager(config) {}
    void setup() override {
        configManager.load();
    }
    void loop() override {}
};

}  // namespace jrb::wifi_serial

