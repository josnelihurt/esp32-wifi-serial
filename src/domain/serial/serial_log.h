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

    /**
     * @brief Appends data to the log buffer.
     *
     * @param data Pointer to the data to append.
     * @param length Length of the data.
     */
    void append(const char* data, int length) override;

    /**
     * @brief Appends a String object to the log buffer.
     *
     * @param data String object to append.
     */
    void append(const String& data) override;

    /**
     * @brief Retrieves new data from the log buffer.
     *
     * @param lastReadPos Reference to an integer that will store the position of the last read byte.
     * @return New data as a String object.
     */
    String getNewData(int& lastReadPos) override;

    /**
     * @brief Resets the log buffer and indices.
     */
    void reset() override;

    /**
     * @brief Checks if there is new data available in the log buffer.
     *
     * @return True if new data is available, false otherwise.
     */
    bool hasData() const override { return hasNewData; }
};

}  // namespace jrb::wifi_serial