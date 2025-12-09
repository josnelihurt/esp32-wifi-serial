#pragma once

#include "config.h"
#include <Preferences.h>
#include <Arduino.h>

namespace jrb::wifi_serial {

class PreferencesStorage final {
private:
    Preferences preferences;
    String m_deviceName;
    String m_mqttBroker;
    int m_mqttPort;
    String m_mqttUser;
    String m_mqttPassword;
    String m_topicTty0Rx;
    String m_topicTty0Tx;
    String m_topicTty1Rx;
    String m_topicTty1Tx;
    
    void generateDefaultTopics();
    void migrateOldTopics();

public:
    PreferencesStorage();
    ~PreferencesStorage() = default;
    
    void load();
    void save();
    void clear();
    
    // Getters
    inline const String& deviceName() const { return m_deviceName; }
    inline const String& mqttBroker() const { return m_mqttBroker; }
    inline int mqttPort() const { return m_mqttPort; }
    inline const String& mqttUser() const { return m_mqttUser; }
    inline const String& mqttPassword() const { return m_mqttPassword; }
    inline const String& topicTty0Rx() const { return m_topicTty0Rx; }
    inline const String& topicTty0Tx() const { return m_topicTty0Tx; }
    inline const String& topicTty1Rx() const { return m_topicTty1Rx; }
    inline const String& topicTty1Tx() const { return m_topicTty1Tx; }
    
    // Setters
    inline void deviceName(const String& v) { m_deviceName = v; }
    inline void mqttBroker(const String& v) { m_mqttBroker = v; }
    inline void mqttPort(int v) { m_mqttPort = v; }
    inline void mqttUser(const String& v) { m_mqttUser = v; }
    inline void mqttPassword(const String& v) { m_mqttPassword = v; }
    inline void topicTty0Rx(const String& v) { m_topicTty0Rx = v; }
    inline void topicTty0Tx(const String& v) { m_topicTty0Tx = v; }
    inline void topicTty1Rx(const String& v) { m_topicTty1Rx = v; }
    inline void topicTty1Tx(const String& v) { m_topicTty1Tx = v; }
};

}  // namespace jrb::wifi_serial
