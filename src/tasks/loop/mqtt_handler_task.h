#pragma once

#include "interfaces/itask.h"
#include "mqtt_client.h"
#include "dependency_container.h"

namespace jrb::wifi_serial {

class MqttHandlerTask final : public ITask {
public:
    MqttHandlerTask() {}
    void loop() override { 
        if (!DependencyContainer::instance) return;
        
        MqttClient* handler = DependencyContainer::instance->getMqttHandler();
        if (handler) {
            handler->loop();
        }
    }
};

}  // namespace jrb::wifi_serial

