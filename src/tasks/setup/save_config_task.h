#pragma once

#include "interfaces/itask.h"
#include "preferences_storage.h"

namespace jrb::wifi_serial {

class SaveConfigTask final : public ITask {
private:
    PreferencesStorage& configManager;

public:
    explicit SaveConfigTask(PreferencesStorage& config) : configManager(config) {}
    void setup() override {
        configManager.save();
    }
    void loop() override {}
};

}  // namespace jrb::wifi_serial

