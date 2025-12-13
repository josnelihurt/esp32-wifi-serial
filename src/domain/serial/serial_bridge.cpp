#include "serial_bridge.h"
#include "config.h"
#include "interfaces/imqtt_client.h"
#include "serial_log.h"
#include <ArduinoLog.h>
namespace jrb::wifi_serial {

SerialBridge::SerialBridge() : serial1{}, buffer0Index{0}, buffer1Index{0} {
  Log.traceln(__PRETTY_FUNCTION__);
}

void SerialBridge::setup(int ttyS1BaudRate) {
  if (ttyS1BaudRate <= 1) {
    Log.errorln("%s: ttyS1BaudRate is %d (<= 1), using default 115200",
               __PRETTY_FUNCTION__, ttyS1BaudRate);
    ttyS1BaudRate = DEFAULT_BAUD_RATE_TTY1;
  }
  Log.infoln("%s: Initializing serial1 with baud rate: %d", __PRETTY_FUNCTION__,
             ttyS1BaudRate);
  serial1 = new HardwareSerial(1);
  serial1->begin(ttyS1BaudRate, SERIAL_8N1, SERIAL1_RX_PIN, SERIAL1_TX_PIN);
  Log.infoln("serial1 initialized successfully");
}

int SerialBridge::readSerial0(char *buffer, int maxLen) {
  Log.traceln(__PRETTY_FUNCTION__);
  if (!available0()) {
    return 0;
  }

  int count = 0;
  while (Serial.available() && count < maxLen - 1) {
    char c = Serial.read();
    if (c != '\0') {
      buffer[count++] = c;
    }
  }
  buffer[count] = '\0';
  return count;
}

int SerialBridge::readSerial1(char *buffer, int maxLen) {
  Log.traceln(__PRETTY_FUNCTION__);
  if (!serial1 || !available1()) {
    return 0;
  }

  int count = 0;
  while (serial1->available() && count < maxLen - 1) {
    char c = serial1->read();
    if (c != '\0') {
      buffer[count++] = c;
    }
  }
  buffer[count] = '\0';
  return count;
}

void SerialBridge::writeSerial0(const char *data, int length) {
  Log.traceln(__PRETTY_FUNCTION__);
  Serial.write((uint8_t *)data, length);
}

void SerialBridge::writeSerial1(const char *data, int length) {
  Log.traceln(__PRETTY_FUNCTION__);
  if (!serial1) {
    Log.errorln("serial1 is not initialized! Cannot write to ttyS1.");
    Log.errorln("Check that baudRateTty1 is set correctly (> 1)");
    return;
  }
  serial1->write((uint8_t *)data, length);
}

bool SerialBridge::available0() const { return Serial.available() > 0; }

bool SerialBridge::available1() const {
  return serial1 && serial1->available() > 0;
}

void SerialBridge::setLogs(SerialLog &log0, SerialLog &log1) {
  Log.traceln(__PRETTY_FUNCTION__);
  serial0Log = &log0;
  serial1Log = &log1;
}

void SerialBridge::setMqttHandler(IMqttClient &client) {
  Log.traceln(__PRETTY_FUNCTION__);
  mqttClient = &client;
}

void SerialBridge::writeToSerialAndLog(int portIndex, const String &serialData,
                                       const String &logData) {
  Log.traceln(__PRETTY_FUNCTION__);
  if (portIndex == 0) {
    writeSerial0(serialData.c_str(), serialData.length());
    if (serial0Log) {
      serial0Log->append(logData);
    }
  } else {
    Log.infoln("$ttyS1(w)$ '%s'", serialData.c_str());
    writeSerial1(serialData.c_str(), serialData.length());
    if (serial1Log) {
      serial1Log->append(logData);
    }
  }
}

void SerialBridge::handleSerialToMqttAndWeb(int portIndex, const char *data,
                                            unsigned int length) {
  Log.traceln(__PRETTY_FUNCTION__);
  if (length == 0 || portIndex < 0 || portIndex > 1)
    return;

  SerialLog *log = (portIndex == 0) ? serial0Log : serial1Log;
  if (log) {
    log->append(data, length);
  }

  if (!mqttClient)
    return;
  mqttClient->appendToBuffer(portIndex, data, length);

  // Also publish visible version for debugging
  String visibleData = makeSpecialCharsVisible(String(data, length));
  String debugMsg = "$serial$ " + visibleData;
  if (portIndex == 0) {
    mqttClient->publishTty0(debugMsg);
  } else {
    mqttClient->publishTty1(debugMsg);
  }
}

void SerialBridge::handleWebToSerialAndMqtt(int portIndex, const String &data) {
  Log.traceln(__PRETTY_FUNCTION__);
  String dataWithNewline = data + "\n";
  String visibleData = makeSpecialCharsVisible(data);
  String webMsg = "$web$ " + visibleData + "\n";

  writeToSerialAndLog(portIndex, dataWithNewline, webMsg);

  if (!mqttClient) {
    return;
  }

  if (!mqttClient->isConnected()) {
    Log.errorln("MQTT client not connected");
    delay(1000);
    return;
  }

  if (portIndex == 0) {
    mqttClient->publishTty0(webMsg);
  } else {
    mqttClient->publishTty1(webMsg);
  }
}

void SerialBridge::handleMqttToSerialAndWeb(int portIndex, const char *data,
                                            unsigned int length) {
  Log.traceln(__PRETTY_FUNCTION__);
  String dataWithNewline = String(data, length) + "\n";
  String visibleData = makeSpecialCharsVisible(String(data, length));
  String mqttMsg = "$mqtt$ " + visibleData + "\n";

  writeToSerialAndLog(portIndex, dataWithNewline, mqttMsg);
}

String SerialBridge::makeSpecialCharsVisible(const String &data) {
  Log.traceln(__PRETTY_FUNCTION__);
  String result;
  result.reserve(data.length() * 3); // Reserve space for expanded characters

  for (size_t i = 0; i < data.length(); i++) {
    char c = data[i];
    switch (c) {
    case '\x00':
      result += "\\0";
      break; // NUL
    case '\x01':
      result += "^A";
      break; // SOH (Ctrl+A)
    case '\x02':
      result += "^B";
      break; // STX
    case '\x03':
      result += "^C";
      break; // ETX (Ctrl+C)
    case '\x04':
      result += "^D";
      break; // EOT (Ctrl+D)
    case '\x05':
      result += "^E";
      break; // ENQ (Ctrl+E)
    case '\x06':
      result += "^F";
      break; // ACK
    case '\x07':
      result += "^G";
      break; // BEL
    case '\x08':
      result += "^H";
      break; // BS (Backspace)
    case '\x09':
      result += "\\t";
      break; // TAB
    case '\x0A':
      result += "\\n";
      break; // LF (Line Feed)
    case '\x0B':
      result += "^K";
      break; // VT
    case '\x0C':
      result += "^L";
      break; // FF
    case '\x0D':
      result += "\\r";
      break; // CR (Carriage Return)
    case '\x0E':
      result += "^N";
      break; // SO
    case '\x0F':
      result += "^O";
      break; // SI
    case '\x10':
      result += "^P";
      break; // DLE
    case '\x11':
      result += "^Q";
      break; // DC1
    case '\x12':
      result += "^R";
      break; // DC2
    case '\x13':
      result += "^S";
      break; // DC3
    case '\x14':
      result += "^T";
      break; // DC4
    case '\x15':
      result += "^U";
      break; // NAK
    case '\x16':
      result += "^V";
      break; // SYN
    case '\x17':
      result += "^W";
      break; // ETB
    case '\x18':
      result += "^X";
      break; // CAN
    case '\x19':
      result += "^Y";
      break; // EM
    case '\x1A':
      result += "^Z";
      break; // SUB (Ctrl+Z)
    case '\x1B':
      result += "\\e";
      break; // ESC
    case '\x1C':
      result += "^\\";
      break; // FS
    case '\x1D':
      result += "^]";
      break; // GS
    case '\x1E':
      result += "^^";
      break; // RS
    case '\x1F':
      result += "^_";
      break; // US
    case '\x7F':
      result += "^?";
      break; // DEL
    default:
      if (c >= 0x20 && c <= 0x7E) {
        result += c; // Printable ASCII
      } else {
        // Non-printable, show as hex
        char hex[5];
        snprintf(hex, sizeof(hex), "\\x%02X", (unsigned char)c);
        result += hex;
      }
      break;
    }
  }

  return result;
}

} // namespace jrb::wifi_serial
