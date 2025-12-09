#include "preferences_storage.h"
#include "config.h"

namespace jrb::wifi_serial {

PreferencesStorage::PreferencesStorage()
    : m_deviceName{DEFAULT_DEVICE_NAME}
    , m_mqttBroker{}
    , m_mqttPort{DEFAULT_MQTT_PORT}
    , m_mqttUser{}
    , m_mqttPassword{}
    , m_topicTty0Rx{}
    , m_topicTty0Tx{}
    , m_topicTty1Rx{}
    , m_topicTty1Tx{}
{
}


void PreferencesStorage::generateDefaultTopics() {
    char baseTopic0[64], baseTopic1[64];
    snprintf(baseTopic0, sizeof(baseTopic0), DEFAULT_TOPIC_TTY0, m_deviceName.c_str());
    snprintf(baseTopic1, sizeof(baseTopic1), DEFAULT_TOPIC_TTY1, m_deviceName.c_str());
    
    String base0 = String(baseTopic0);
    String base1 = String(baseTopic1);
    
    if (!base0.startsWith("wifi_serial/")) {
        base0 = "wifi_serial/" + base0;
    }
    if (!base1.startsWith("wifi_serial/")) {
        base1 = "wifi_serial/" + base1;
    }
    
    if (m_topicTty0Rx.length() == 0) {
        m_topicTty0Rx = base0 + "/rx";
    } else if (!m_topicTty0Rx.startsWith("wifi_serial/")) {
        m_topicTty0Rx = "wifi_serial/" + m_topicTty0Rx;
    }
    
    if (m_topicTty0Tx.length() == 0) {
        m_topicTty0Tx = base0 + "/tx";
    } else if (!m_topicTty0Tx.startsWith("wifi_serial/")) {
        m_topicTty0Tx = "wifi_serial/" + m_topicTty0Tx;
    }
    
    if (m_topicTty1Rx.length() == 0) {
        m_topicTty1Rx = base1 + "/rx";
    } else if (!m_topicTty1Rx.startsWith("wifi_serial/")) {
        m_topicTty1Rx = "wifi_serial/" + m_topicTty1Rx;
    }
    
    if (m_topicTty1Tx.length() == 0) {
        m_topicTty1Tx = base1 + "/tx";
    } else if (!m_topicTty1Tx.startsWith("wifi_serial/")) {
        m_topicTty1Tx = "wifi_serial/" + m_topicTty1Tx;
    }
}

void PreferencesStorage::migrateOldTopics() {
    preferences.begin("esp32bridge", true);
    String oldTopic0 = preferences.getString("topicTty0", "");
    String oldTopic1 = preferences.getString("topicTty1", "");
    preferences.end();
    
    if (oldTopic0.length() > 0 && m_topicTty0Rx.length() == 0) {
        String base = oldTopic0;
        if (base.endsWith("/rx") || base.endsWith("/tx")) {
            int lastSlash = base.lastIndexOf('/');
            base = base.substring(0, lastSlash);
        }
        m_topicTty0Rx = base + "/rx";
        m_topicTty0Tx = base + "/tx";
    }
    
    if (oldTopic1.length() > 0 && m_topicTty1Rx.length() == 0) {
        String base = oldTopic1;
        if (base.endsWith("/rx") || base.endsWith("/tx")) {
            int lastSlash = base.lastIndexOf('/');
            base = base.substring(0, lastSlash);
        }
        m_topicTty1Rx = base + "/rx";
        m_topicTty1Tx = base + "/tx";
    }
}

void PreferencesStorage::load() {
    preferences.begin("esp32bridge", true);
    
    m_deviceName = preferences.getString("deviceName", DEFAULT_DEVICE_NAME);
    m_mqttBroker = preferences.getString("mqttBroker", "");
    m_mqttPort = preferences.getInt("mqttPort", DEFAULT_MQTT_PORT);
    m_mqttUser = preferences.getString("mqttUser", "");
    m_mqttPassword = preferences.getString("mqttPassword", "");
    
    m_topicTty0Rx = preferences.getString("topicTty0Rx", "");
    m_topicTty0Tx = preferences.getString("topicTty0Tx", "");
    m_topicTty1Rx = preferences.getString("topicTty1Rx", "");
    m_topicTty1Tx = preferences.getString("topicTty1Tx", "");
    
    preferences.end();
    
    migrateOldTopics();
    generateDefaultTopics();
}

void PreferencesStorage::save() {
    preferences.begin("esp32bridge", false);
    
    preferences.putString("deviceName", m_deviceName);
    preferences.putString("mqttBroker", m_mqttBroker);
    preferences.putInt("mqttPort", m_mqttPort);
    preferences.putString("mqttUser", m_mqttUser);
    preferences.putString("mqttPassword", m_mqttPassword);
    preferences.putString("topicTty0Rx", m_topicTty0Rx);
    preferences.putString("topicTty0Tx", m_topicTty0Tx);
    preferences.putString("topicTty1Rx", m_topicTty1Rx);
    preferences.putString("topicTty1Tx", m_topicTty1Tx);
    
    preferences.end();
}

void PreferencesStorage::clear() {
    preferences.begin("esp32bridge", false);
    preferences.clear();
    preferences.end();
    
    m_deviceName = DEFAULT_DEVICE_NAME;
    m_mqttBroker = "";
    m_mqttPort = DEFAULT_MQTT_PORT;
    m_mqttUser = "";
    m_mqttPassword = "";
    m_topicTty0Rx = "";
    m_topicTty0Tx = "";
    m_topicTty1Rx = "";
    m_topicTty1Tx = "";
}

}  // namespace jrb::wifi_serial

