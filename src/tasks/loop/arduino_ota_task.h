#pragma once

#include "interfaces/itask.h"
#include <ArduinoOTA.h>

namespace jrb::wifi_serial {

class ArduinoOTATask final : public ITask {
public:
    void loop() override { ArduinoOTA.handle(); }
};

}  // namespace jrb::wifi_serial

