#include "serial_log.h"

namespace jrb::wifi_serial {

SerialLog::SerialLog()
    : writeIndex{0}
    , readIndex{0}
    , hasNewData{false}
{
    memset(buffer, 0, SERIAL_LOG_SIZE);
}

void SerialLog::append(const char* data, int length) {
    for (int i = 0; i < length; i++) {
        buffer[writeIndex] = data[i];
        writeIndex = (writeIndex + 1) % SERIAL_LOG_SIZE;
        if (writeIndex == readIndex) {
            readIndex = (readIndex + 1) % SERIAL_LOG_SIZE;
        }
    }
    hasNewData = true;
}

void SerialLog::append(const String& data) {
    append(data.c_str(), data.length());
}

String SerialLog::getNewData(int& lastReadPos) {
    if (!hasNewData) {
        return "";
    }
    
    int startPos = lastReadPos % SERIAL_LOG_SIZE;
    int endPos = writeIndex;
    
    if (endPos == startPos) {
        return "";
    }
    
    String result = "";
    
    if (endPos > startPos) {
        int len = endPos - startPos;
        char* temp = new char[len + 1];
        memcpy(temp, buffer + startPos, len);
        temp[len] = '\0';
        result = String(temp);
        delete[] temp;
        lastReadPos = endPos;
    } else {
        int len1 = SERIAL_LOG_SIZE - startPos;
        int len2 = endPos;
        int totalLen = len1 + len2;
        char* temp = new char[totalLen + 1];
        memcpy(temp, buffer + startPos, len1);
        memcpy(temp + len1, buffer, len2);
        temp[totalLen] = '\0';
        result = String(temp);
        delete[] temp;
        lastReadPos = endPos;
    }
    
    if (lastReadPos == writeIndex) {
        hasNewData = false;
    }
    
    return result;
}

void SerialLog::reset() {
    readIndex = writeIndex;
    hasNewData = false;
}

}  // namespace jrb::wifi_serial

