#pragma once

#include "interfaces/itask.h"
#include "domain/network/wifi_manager.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

class WiFiConfigAPModeWaitTask final : public ITask {
private:
    WiFiManager& wifiManager;

public:
    explicit WiFiConfigAPModeWaitTask(WiFiManager& wifi) : wifiManager(wifi) {}
    
    void setup() override;
    void loop() override {}
};

}  // namespace jrb::wifi_serial

