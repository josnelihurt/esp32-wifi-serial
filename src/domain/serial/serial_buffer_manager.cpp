#include "serial_buffer_manager.h"
#include <ArduinoLog.h>

namespace jrb::wifi_serial {

SerialBufferManagerDeprecated::SerialBufferManagerDeprecated()
    : bufferLengths{0, 0}
{
    memset(buffers, 0, sizeof(buffers));
    memset(metrics, 0, sizeof(metrics));
}

int SerialBufferManagerDeprecated::readFromSerial(Stream& serial, int portIndex) {
    Log.infoln(__PRETTY_FUNCTION__);
    if (portIndex < 0 || portIndex >= NUM_PORTS) return 0;

    int count = 0;
    int available = serial.available();

    // Track metrics
    if (available > (int)metrics[portIndex].peakBufferUsage) {
        metrics[portIndex].peakBufferUsage = available;
    }

    // Detect overflow
    if (available > BUFFER_SIZE - 1) {
        metrics[portIndex].overflowEvents++;
        metrics[portIndex].bytesDropped += (available - (BUFFER_SIZE - 1));
        Log.warningln("Buffer overflow on port %d: %d bytes available",
                     portIndex, available);
    }

    while (serial.available() && count < BUFFER_SIZE - 1) {
        char c = serial.read();
        if (c != '\0') {
            Serial.print(c);Serial.flush();
            buffers[portIndex][count++] = c;
        }
    }

    buffers[portIndex][count] = '\0';
    bufferLengths[portIndex] = count;
    metrics[portIndex].bytesReceived += count;

    return count;
}

const char* SerialBufferManagerDeprecated::getBuffer(int portIndex, int& length) const {
    if (portIndex < 0 || portIndex >= NUM_PORTS) {
        length = 0;
        return nullptr;
    }
    length = bufferLengths[portIndex];
    return buffers[portIndex];
}

void SerialBufferManagerDeprecated::clearBuffer(int portIndex) {
    if (portIndex >= 0 && portIndex < NUM_PORTS) {
        bufferLengths[portIndex] = 0;
    }
}

SerialBufferManagerDeprecated::BufferMetrics SerialBufferManagerDeprecated::getMetrics(int portIndex) const {
    if (portIndex >= 0 && portIndex < NUM_PORTS) {
        return metrics[portIndex];
    }
    return BufferMetrics{};
}

void SerialBufferManagerDeprecated::resetMetrics(int portIndex) {
    if (portIndex >= 0 && portIndex < NUM_PORTS) {
        metrics[portIndex] = BufferMetrics{};
    }
}

}  // namespace jrb::wifi_serial
