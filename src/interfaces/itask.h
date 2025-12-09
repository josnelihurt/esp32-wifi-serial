#pragma once

namespace jrb::wifi_serial {

class ITask {
public:
    virtual ~ITask() = default;
    virtual void setup() {}
    virtual void loop() = 0;
};

}  // namespace jrb::wifi_serial

