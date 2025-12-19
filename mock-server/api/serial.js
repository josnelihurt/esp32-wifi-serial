/**
 * Serial communication API endpoints
 * Mimics ESP32 serial polling and sending behavior
 */

const express = require('express');
const router = express.Router();

// Simulated serial buffers (referenced from mockData in server.js)
let mockData = null;

// Initialize with mock data reference
function init(data) {
  mockData = data;
}

// Simulate random serial data generation for testing
function generateRandomSerialData(port) {
  const samples = [
    'System initialized\n',
    'Temperature: 25.3C\n',
    'Sensor reading: 1024\n',
    'Network status: OK\n',
    'DEBUG: Processing command\n',
    '',  // Empty most of the time
    '',
    '',
    ''
  ];

  // 10% chance of generating data
  if (Math.random() < 0.1) {
    const data = samples[Math.floor(Math.random() * samples.length)];
    if (data) {
      const bufferKey = port === 0 ? 'serial0Buffer' : 'serial1Buffer';
      mockData[bufferKey].push(data);
    }
  }
}

// GET /serial0/poll - Poll serial port 0
router.get('/serial0/poll', (req, res) => {
  generateRandomSerialData(0);

  if (mockData.serial0Buffer.length > 0) {
    const data = mockData.serial0Buffer.join('');
    mockData.serial0Buffer = [];
    res.status(200).type('text/plain').send(data);
  } else {
    res.status(200).type('text/plain').send('');
  }
});

// GET /serial1/poll - Poll serial port 1
router.get('/serial1/poll', (req, res) => {
  generateRandomSerialData(1);

  if (mockData.serial1Buffer.length > 0) {
    const data = mockData.serial1Buffer.join('');
    mockData.serial1Buffer = [];
    res.status(200).type('text/plain').send(data);
  } else {
    res.status(200).type('text/plain').send('');
  }
});

// POST /serial0/send - Send data to serial port 0
router.post('/serial0/send', (req, res) => {
  const data = req.body.data;

  if (!data) {
    return res.status(400).type('text/plain').send('Missing data');
  }

  console.log(`[Serial0 TX] ${data.replace(/\r/g, '\\r').replace(/\n/g, '\\n')}`);

  // Simulate echo back after delay
  setTimeout(() => {
    mockData.serial0Buffer.push(`Echo: ${data}`);
  }, 100);

  res.status(200).type('text/plain').send('OK');
});

// POST /serial1/send - Send data to serial port 1
router.post('/serial1/send', (req, res) => {
  const data = req.body.data;

  if (!data) {
    return res.status(400).type('text/plain').send('Missing data');
  }

  console.log(`[Serial1 TX] ${data.replace(/\r/g, '\\r').replace(/\n/g, '\\n')}`);

  // Simulate echo back after delay
  setTimeout(() => {
    mockData.serial1Buffer.push(`Echo: ${data}`);
  }, 100);

  res.status(200).type('text/plain').send('OK');
});

module.exports = {
  router,
  init
};
