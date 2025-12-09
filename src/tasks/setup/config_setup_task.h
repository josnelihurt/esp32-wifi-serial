#pragma once

#include "interfaces/itask.h"
#include "preferences_storage.h"
#include "system_info.h"

namespace jrb::wifi_serial {

class ConfigSetupTask final : public ITask {
private:
    PreferencesStorage& preferencesStorage;
    SystemInfo& systemInfo;

public:
    ConfigSetupTask(PreferencesStorage& storage, SystemInfo& info)
        : preferencesStorage(storage), systemInfo(info) {}
    
    void setup() override {
        preferencesStorage.load();
        systemInfo.printWelcomeMessage();
    }
    
    void loop() override {}
};

}  // namespace jrb::wifi_serial

