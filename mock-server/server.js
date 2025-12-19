/**
 * ESP32 WiFi Serial - Mock Development Server
 *
 * This server simulates the ESP32's web server behavior for local frontend development
 * It serves files from the ../data directory and processes template variables
 * All API endpoints are mocked to behave like the real ESP32
 */

const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const multer = require('multer');
const path = require('path');
const fs = require('fs');

// Import middleware
const basicAuth = require('./middleware/auth');
const { processTemplate } = require('./middleware/template');

// Import API routes
const serialAPI = require('./api/serial');
const configAPI = require('./api/config');
const otaAPI = require('./api/ota');

// Configuration
const PORT = process.env.PORT || 3000;
const DATA_DIR = path.join(__dirname, '../data');
const CONFIG_FILE = path.join(__dirname, 'config/mock-data.json');

// Load mock data
let mockData = JSON.parse(fs.readFileSync(CONFIG_FILE, 'utf8'));

// Initialize API modules with mock data
serialAPI.init(mockData);
configAPI.init(mockData);
otaAPI.init(mockData);

// Create Express app
const app = express();

// Configure multer for file uploads (OTA)
const upload = multer({ storage: multer.memoryStorage() });

// Middleware
app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

// Logging middleware
app.use((req, res, next) => {
  console.log(`[${new Date().toISOString()}] ${req.method} ${req.path}`);
  next();
});

// Serve static files (CSS, JS, SVG) without authentication or template processing
app.get('/style.css', (req, res) => {
  res.sendFile(path.join(DATA_DIR, 'style.css'));
});

app.get('/script.js', (req, res) => {
  res.sendFile(path.join(DATA_DIR, 'script.js'));
});

app.get('/favicon.svg', (req, res) => {
  res.sendFile(path.join(DATA_DIR, 'favicon.svg'));
});

// Serve index.html with authentication and template processing
app.get('/', basicAuth(mockData), (req, res) => {
  const indexPath = path.join(DATA_DIR, 'index.html');
  fs.readFile(indexPath, 'utf8', (err, data) => {
    if (err) {
      console.error('[Server] Error reading index.html:', err);
      return res.status(500).send('Error loading page');
    }
    const processed = processTemplate(data, mockData);
    res.type('text/html').send(processed);
  });
});

// Serve OTA HTML with authentication and template processing
app.get('/ota.html', basicAuth(mockData), (req, res) => {
  const otaPath = path.join(DATA_DIR, 'ota.html');
  fs.readFile(otaPath, 'utf8', (err, data) => {
    if (err) {
      console.error('[Server] Error reading ota.html:', err);
      return res.status(500).send('Error loading page');
    }
    const processed = processTemplate(data, mockData);
    res.type('text/html').send(processed);
  });
});

// Serve OTA Filesystem HTML with authentication and template processing
app.get('/ota-fs.html', basicAuth(mockData), (req, res) => {
  const otaFsPath = path.join(DATA_DIR, 'ota-fs.html');
  fs.readFile(otaFsPath, 'utf8', (err, data) => {
    if (err) {
      console.error('[Server] Error reading ota-fs.html:', err);
      return res.status(500).send('Error loading page');
    }
    const processed = processTemplate(data, mockData);
    res.type('text/html').send(processed);
  });
});

// Serve About HTML with authentication and template processing
app.get('/about.html', basicAuth(mockData), (req, res) => {
  const aboutPath = path.join(DATA_DIR, 'about.html');
  fs.readFile(aboutPath, 'utf8', (err, data) => {
    if (err) {
      console.error('[Server] Error reading about.html:', err);
      return res.status(500).send('Error loading page');
    }
    const processed = processTemplate(data, mockData);
    res.type('text/html').send(processed);
  });
});

// Mount API routes with authentication
app.use('/serial0', basicAuth(mockData), serialAPI.router);
app.use('/serial1', basicAuth(mockData), serialAPI.router);
app.use('/', basicAuth(mockData), configAPI.router);
app.use('/ota', basicAuth(mockData), otaAPI.router);

// OTA file upload endpoints need special handling with multer
app.post('/ota/firmware/upload', basicAuth(mockData), upload.single('file'), otaAPI.router);
app.post('/ota/filesystem/upload', basicAuth(mockData), upload.single('file'), otaAPI.router);

// Start server
app.listen(PORT, () => {
  console.log('========================================');
  console.log('ESP32 WiFi Serial - Mock Development Server');
  console.log('========================================');
  console.log(`Server running at: http://localhost:${PORT}`);
  console.log(`Data directory: ${DATA_DIR}`);
  console.log('');
  console.log('Authentication:');
  console.log(`  Username: ${mockData.webUser}`);
  console.log(`  Password: ${mockData.webPassword}`);
  console.log('');
  console.log('Mock Configuration:');
  console.log(`  Device Name: ${mockData.deviceName}`);
  console.log(`  WiFi SSID: ${mockData.ssid}`);
  console.log(`  MQTT Broker: ${mockData.mqttBroker}:${mockData.mqttPort}`);
  console.log('');
  console.log('Press Ctrl+C to stop');
  console.log('========================================');
});
