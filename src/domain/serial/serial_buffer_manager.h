#pragma once

#include "config.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

class SerialBufferManagerDeprecated final {
public:
    struct BufferMetrics {
        uint32_t bytesReceived;
        uint32_t bytesDropped;
        uint32_t peakBufferUsage;
        uint32_t overflowEvents;
    };

    SerialBufferManagerDeprecated();

    // Read from serial into managed buffer
    int readFromSerial(Stream& serial, int portIndex);

    // Get buffer contents
    const char* getBuffer(int portIndex, int& length) const;

    // Clear after processing
    void clearBuffer(int portIndex);

    // Metrics
    BufferMetrics getMetrics(int portIndex) const;
    void resetMetrics(int portIndex);

private:
    static constexpr int BUFFER_SIZE = 512;
    static constexpr int NUM_PORTS = 2;

    char buffers[NUM_PORTS][BUFFER_SIZE];
    int bufferLengths[NUM_PORTS];
    BufferMetrics metrics[NUM_PORTS];
};

}  // namespace jrb::wifi_serial
