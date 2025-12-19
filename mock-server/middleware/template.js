/**
 * Template processor middleware
 * Mimics ESP32 template processing from web_config_server.cpp:299-367
 * Replaces %VARIABLE% placeholders with mock data
 */

function escapeHTML(str) {
  if (!str) return '';
  return str
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#39;');
}

function processTemplate(content, mockData) {
  let processed = content;

  // WiFi Configuration
  processed = processed.replace(/%SSID%/g, escapeHTML(mockData.ssid));
  processed = processed.replace(/%PASSWORD_DISPLAY%/g, mockData.password ? '********' : '');
  processed = processed.replace(/%PASSWORD_HAS_VALUE%/g, mockData.password ? '1' : '0');

  // Device Configuration
  processed = processed.replace(/%DEVICE_NAME%/g, escapeHTML(mockData.deviceName || 'esp32c3'));

  // MQTT Configuration
  processed = processed.replace(/%MQTT_BROKER%/g, escapeHTML(mockData.mqttBroker));
  processed = processed.replace(/%MQTT_PORT%/g, String(mockData.mqttPort || 1883));
  processed = processed.replace(/%MQTT_USER%/g, escapeHTML(mockData.mqttUser));
  processed = processed.replace(/%MQTT_PASSWORD_DISPLAY%/g, mockData.mqttPassword ? '********' : '');
  processed = processed.replace(/%MQTT_PASSWORD_HAS_VALUE%/g, mockData.mqttPassword ? '1' : '0');

  // MQTT Topics
  processed = processed.replace(/%TOPIC_TTY0_RX%/g, escapeHTML(mockData.topicTty0Rx));
  processed = processed.replace(/%TOPIC_TTY0_TX%/g, escapeHTML(mockData.topicTty0Tx));
  processed = processed.replace(/%TOPIC_TTY1_RX%/g, escapeHTML(mockData.topicTty1Rx));
  processed = processed.replace(/%TOPIC_TTY1_TX%/g, escapeHTML(mockData.topicTty1Tx));

  // Baud Rate
  processed = processed.replace(/%BAUD_RATE_TTY1%/g, String(mockData.baudRateTty1));

  // IP Address
  processed = processed.replace(/%IP_ADDRESS%/g, mockData.ipAddress);

  // Web User Configuration
  processed = processed.replace(/%WEB_USER%/g, escapeHTML(mockData.webUser));
  processed = processed.replace(/%WEB_PASSWORD_DISPLAY%/g, mockData.webPassword ? '********' : '');
  processed = processed.replace(/%WEB_PASSWORD_HAS_VALUE%/g, mockData.webPassword ? '1' : '0');

  // OTA Configuration
  processed = processed.replace(
    /%OTA_PASSWORD_STATUS%/g,
    mockData.webPassword
      ? 'Configured (uses Web Password)'
      : 'Not configured - OTA is unprotected!'
  );

  return processed;
}

module.exports = {
  processTemplate,
  escapeHTML
};
