#pragma once

#include "config.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

class SerialLog final {
private:
    char buffer[SERIAL_LOG_SIZE];
    int writeIndex;
    int readIndex;
    bool hasNewData;

public:
    SerialLog();
    void append(const char* data, int length);
    void append(const String& data);
    String getNewData(int& lastReadPos);
    void reset();
    bool hasData() const { return hasNewData; }
};

}  // namespace jrb::wifi_serial

