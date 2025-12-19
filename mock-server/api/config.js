/**
 * Configuration API endpoints
 * Mimics ESP32 configuration save and reset behavior
 */

const express = require('express');
const router = express.Router();
const fs = require('fs');
const path = require('path');

let mockData = null;
const configPath = path.join(__dirname, '../config/mock-data.json');

// Initialize with mock data reference
function init(data) {
  mockData = data;
}

// POST /save - Save configuration
router.post('/save', (req, res) => {
  console.log('[Config] Saving configuration:', req.body);

  // Update baud rate
  if (req.body.speed0) {
    const baudRate = parseInt(req.body.speed0);
    if (baudRate <= 1) {
      console.warn(`[Config] Invalid baud rate ${baudRate}, using default 115200`);
      mockData.baudRateTty1 = 115200;
    } else {
      mockData.baudRateTty1 = baudRate;
    }
  }

  // Update WiFi settings
  if (req.body.ssid) {
    mockData.ssid = req.body.ssid;
  }
  if (req.body.password && req.body.password !== '********' && req.body.password.length > 0) {
    mockData.password = req.body.password;
  }

  // Update device name and MQTT topics
  if (req.body.device) {
    mockData.deviceName = req.body.device;
    mockData.topicTty0Rx = `wifi_serial/${mockData.deviceName}/ttyS0/rx`;
    mockData.topicTty0Tx = `wifi_serial/${mockData.deviceName}/ttyS0/tx`;
    mockData.topicTty1Rx = `wifi_serial/${mockData.deviceName}/ttyS1/rx`;
    mockData.topicTty1Tx = `wifi_serial/${mockData.deviceName}/ttyS1/tx`;
  }

  // Update MQTT settings
  if (req.body.broker) {
    mockData.mqttBroker = req.body.broker;
  }
  if (req.body.port) {
    mockData.mqttPort = parseInt(req.body.port);
  }
  if (req.body.user) {
    mockData.mqttUser = req.body.user;
  }
  if (req.body.mqttpass && req.body.mqttpass !== '********' && req.body.mqttpass.length > 0) {
    mockData.mqttPassword = req.body.mqttpass;
  }

  // Update Web User settings
  if (req.body.web_user) {
    mockData.webUser = req.body.web_user;
  }
  if (req.body.web_password && req.body.web_password !== '********' && req.body.web_password.length > 0) {
    mockData.webPassword = req.body.web_password;
  }

  // Save to file
  try {
    fs.writeFileSync(configPath, JSON.stringify(mockData, null, 2));
    console.log('[Config] Configuration saved to file');
  } catch (err) {
    console.error('[Config] Error saving configuration:', err);
  }

  res.status(200).type('text/plain').send('Configuration saved! Restarting...');

  // Simulate restart delay (in real ESP32, it would restart)
  console.log('[Config] Simulating device restart...');
});

// POST /reset - Reset device
router.post('/reset', (req, res) => {
  console.log('[Config] Device reset requested');
  res.status(200).type('text/plain').send('Device resetting...');

  // In real ESP32, this would restart the device
  console.log('[Config] Simulating device reset...');
});

module.exports = {
  router,
  init
};
