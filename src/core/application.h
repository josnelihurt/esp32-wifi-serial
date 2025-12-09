#pragma once

#include "dependency_container.h"
#include "task_registry.h"
#include "serial_bridge.h"
#include <Arduino.h>
#include <functional>

namespace jrb::wifi_serial {

class Application final {
public:
    explicit Application(DependencyContainer& container);
    ~Application() = default;
    
    void initialize();
    void setup();
    void loop();
    
    void onMqttTty0(const char* data, unsigned int length);
    void onMqttTty1(const char* data, unsigned int length);
    void onResetConfig();
    void handleWebToSerialAndMqtt(int portIndex, const String& data);
    
    std::function<void(const char*, unsigned int)> getMqttTty0Callback();
    std::function<void(const char*, unsigned int)> getMqttTty1Callback();
    std::function<void()> getResetConfigCallback();

private:
    DependencyContainer& container;
    TaskRegistry registry;
    
    void registerSetupTasks();
    void registerLoopTasks();
};

}  // namespace jrb::wifi_serial

