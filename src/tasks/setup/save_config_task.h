#pragma once

#include "interfaces/itask.h"
#include "domain/config/preferences_storage.h"

namespace jrb::wifi_serial {

class SaveConfigTask final : public ITask {
private:
    PreferencesStorage& preferencesStorage;

public:
    explicit SaveConfigTask(PreferencesStorage& storage) : preferencesStorage(storage) {}
    void setup() override {
        preferencesStorage.save();
    }
    void loop() override {}
};

}  // namespace jrb::wifi_serial

