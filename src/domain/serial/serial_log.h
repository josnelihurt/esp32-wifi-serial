#pragma once

#include "config.h"
#include "interfaces/iserial_log.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

class SerialLog final : public ISerialLog {
private:
    char buffer[SERIAL_LOG_SIZE];
    int writeIndex;
    int readIndex;
    bool hasNewData;

public:
    SerialLog();
    void append(const char* data, int length) override;
    void append(const String& data) override;
    String getNewData(int& lastReadPos) override;
    void reset() override;
    bool hasData() const override { return hasNewData; }
};

}  // namespace jrb::wifi_serial

