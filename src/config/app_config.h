#pragma once

namespace jrb::wifi_serial {

#define TRIPLE_PRESS_TIMEOUT 2000

#define SERIAL_BUFFER_SIZE 256
#define SERIAL_LOG_SIZE 4096

#define CMD_PREFIX 0x19  // Ctrl+Y
#define CMD_INFO 'i'
#define CMD_DEBUG 'd'

}  // namespace jrb::wifi_serial

