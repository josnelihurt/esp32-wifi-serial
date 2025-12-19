/**
 * OTA (Over-The-Air) update API endpoints
 * Mimics ESP32 OTA firmware and filesystem upload behavior
 */

const express = require('express');
const router = express.Router();
const crypto = require('crypto');

let mockData = null;
let otaInProgress = false;
let otaReceivedSize = 0;
let otaExpectedSize = 0;

// Initialize with mock data reference
function init(data) {
  mockData = data;
}

// GET /ota/status - Get OTA status
router.get('/ota/status', (req, res) => {
  const status = {
    otaInProgress: otaInProgress,
    receivedSize: otaReceivedSize,
    expectedSize: otaExpectedSize
  };
  res.status(200).json(status);
});

// POST /ota/firmware/upload - Upload firmware
router.post('/ota/firmware/upload', (req, res) => {
  if (!req.file) {
    return res.status(400).type('text/plain').send('No file uploaded');
  }

  const file = req.file;
  const expectedHash = req.body.hash;

  console.log('[OTA Firmware] Upload started:', file.originalname, 'Size:', file.size, 'bytes');

  // Validate filename
  if (!file.originalname.endsWith('.bin')) {
    console.error('[OTA Firmware] Invalid file extension');
    return res.status(400).type('text/plain').send('Invalid firmware file extension');
  }

  // Calculate SHA256 hash
  const hash = crypto.createHash('sha256');
  hash.update(file.buffer);
  const calculatedHash = hash.digest('hex');

  console.log('[OTA Firmware] Expected hash:', expectedHash);
  console.log('[OTA Firmware] Calculated hash:', calculatedHash);

  // Verify hash if provided
  if (expectedHash && expectedHash.length > 0 && calculatedHash !== expectedHash) {
    console.error('[OTA Firmware] Hash verification failed!');
    return res.status(400).type('text/plain').send('Hash verification failed');
  }

  console.log('[OTA Firmware] Upload completed successfully');
  console.log('[OTA Firmware] Simulating device restart...');

  res.status(200).type('text/plain').send('Firmware updated successfully');
});

// POST /ota/filesystem/upload - Upload filesystem
router.post('/ota/filesystem/upload', (req, res) => {
  if (!req.file) {
    return res.status(400).type('text/plain').send('No file uploaded');
  }

  const file = req.file;
  const expectedHash = req.body.hash;

  console.log('[OTA Filesystem] Upload started:', file.originalname, 'Size:', file.size, 'bytes');

  // Calculate SHA256 hash
  const hash = crypto.createHash('sha256');
  hash.update(file.buffer);
  const calculatedHash = hash.digest('hex');

  console.log('[OTA Filesystem] Expected hash:', expectedHash);
  console.log('[OTA Filesystem] Calculated hash:', calculatedHash);

  // Verify hash if provided
  if (expectedHash && expectedHash.length > 0 && calculatedHash !== expectedHash) {
    console.error('[OTA Filesystem] Hash verification failed!');
    return res.status(400).type('text/plain').send('Hash verification failed');
  }

  console.log('[OTA Filesystem] Upload completed successfully');
  console.log('[OTA Filesystem] Simulating device restart...');

  res.status(200).type('text/plain').send('Filesystem updated successfully');
});

module.exports = {
  router,
  init
};
