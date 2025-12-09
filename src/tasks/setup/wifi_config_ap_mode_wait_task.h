#pragma once

#include "interfaces/itask.h"
#include "wifi_manager.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

class WiFiConfigAPModeWaitTask final : public ITask {
private:
    WiFiManager& wifiConfig;

public:
    explicit WiFiConfigAPModeWaitTask(WiFiManager& config) : wifiConfig(config) {}
    
    void setup() override;
    void loop() override {}
};

}  // namespace jrb::wifi_serial

