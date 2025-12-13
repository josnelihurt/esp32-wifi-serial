#pragma once

#include <Arduino.h>

namespace jrb::wifi_serial {

class SerialLog;
class IMqttClient;

/**
 * @brief Interface for serial bridge operations
 * 
 * Provides abstraction for bridging serial port data between hardware serial ports,
 * MQTT, and web interfaces. Handles bidirectional communication for two serial ports.
 */
class ISerialBridge {
public:
    virtual ~ISerialBridge() = default;
};

}  // namespace jrb::wifi_serial

