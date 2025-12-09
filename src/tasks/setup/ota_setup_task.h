#pragma once

#include "interfaces/itask.h"
#include "ota_manager.h"

namespace jrb::wifi_serial {

class OTASetupTask final : public ITask {
private:
    OTAManager& otaManager;

public:
    explicit OTASetupTask(OTAManager& ota) : otaManager(ota) {}
    void setup() override {
        otaManager.setup();
    }
    void loop() override {}
};

}  // namespace jrb::wifi_serial

