#pragma once

#include "interfaces/itask.h"
#include "wifi_manager.h"

namespace jrb::wifi_serial {

class WiFiConfigTask final : public ITask {
private:
    WiFiManager& wifiConfig;

public:
    explicit WiFiConfigTask(WiFiManager& config) : wifiConfig(config) {}
    void loop() override { wifiConfig.loop(); }
};

}  // namespace jrb::wifi_serial

