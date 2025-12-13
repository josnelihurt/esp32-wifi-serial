#include "serial_command_handler.h"
#include "config.h"
#include <ArduinoLog.h>

namespace jrb::wifi_serial {

void SerialCommandHandler::handle() {
    if (!Serial.available()) return;
    
    int c = Serial.read();
    
    if (c == CMD_PREFIX) {
        cmdPrefixReceived = true;
        return;
    }
    
    if (!cmdPrefixReceived) return;
    
    cmdPrefixReceived = false;
    
    if (c == CMD_INFO || c == 'I') {
        if (printWelcomeFunc) {
            printWelcomeFunc();
        }
        return;
    }
    
    if (c == CMD_DEBUG || c == 'D') {
        debugEnabled = !debugEnabled;
        Log.infoln("Debug %s", debugEnabled ? "enabled" : "disabled");
    }
}

}  // namespace jrb::wifi_serial

