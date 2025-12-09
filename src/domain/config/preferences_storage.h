#pragma once

#include "config.h"
#include "interfaces/istorage.h"
#include <Preferences.h>
#include <Arduino.h>

namespace jrb::wifi_serial {

class PreferencesStorage final : public IStorage {
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
    
    void load() override;
    void save() override;
    void clear() override;
    
    const String& deviceName() const override { return m_deviceName; }
    const String& mqttBroker() const override { return m_mqttBroker; }
    int mqttPort() const override { return m_mqttPort; }
    const String& mqttUser() const override { return m_mqttUser; }
    const String& mqttPassword() const override { return m_mqttPassword; }
    const String& topicTty0Rx() const override { return m_topicTty0Rx; }
    const String& topicTty0Tx() const override { return m_topicTty0Tx; }
    const String& topicTty1Rx() const override { return m_topicTty1Rx; }
    const String& topicTty1Tx() const override { return m_topicTty1Tx; }
    
    void deviceName(const String& v) override { m_deviceName = v; }
    void mqttBroker(const String& v) override { m_mqttBroker = v; }
    void mqttPort(int v) override { m_mqttPort = v; }
    void mqttUser(const String& v) override { m_mqttUser = v; }
    void mqttPassword(const String& v) override { m_mqttPassword = v; }
    void topicTty0Rx(const String& v) override { m_topicTty0Rx = v; }
    void topicTty0Tx(const String& v) override { m_topicTty0Tx = v; }
    void topicTty1Rx(const String& v) override { m_topicTty1Rx = v; }
    void topicTty1Tx(const String& v) override { m_topicTty1Tx = v; }
};

}  // namespace jrb::wifi_serial
