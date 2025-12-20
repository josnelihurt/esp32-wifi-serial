let lastPos0 = 0;
let lastPos1 = 0;
let polling0 = false;
let polling1 = false;
let otaFirmwareLoaded = false;
let otaFilesystemLoaded = false;
let aboutLoaded = false;
let localEcho0 = false;
let localEcho1 = false;
let passwordMode0 = false;
let passwordMode1 = false;
let autoNewline0 = false;
let autoNewline1 = false;

function switchTab(index) {
    document.querySelectorAll('.tab').forEach((t, i) => {
        t.classList.toggle('active', i === index);
    });
    document.querySelectorAll('.tab-content').forEach((c, i) => {
        c.classList.toggle('active', i === index);
    });

    if (index === 1 && !polling0) {
        polling0 = true;
        pollSerial(0);
    } else if (index !== 1) {
        polling0 = false;
    }

    if (index === 2 && !polling1) {
        polling1 = true;
        pollSerial(1);
    } else if (index !== 2) {
        polling1 = false;
    }

    if (index === 3 && !otaFirmwareLoaded) {
        loadOTAFirmwareContent();
    }

    if (index === 4 && !otaFilesystemLoaded) {
        loadOTAFilesystemContent();
    }

    if (index === 5 && !aboutLoaded) {
        loadAboutContent();
    }
}

function loadOTAFirmwareContent() {
    fetch('/ota.html')
        .then(r => r.text())
        .then(html => {
            const otaContent = document.getElementById('ota-content');
            if (otaContent) {
                otaContent.innerHTML = html;
                otaFirmwareLoaded = true;
            }
        })
        .catch(e => {
            const otaContent = document.getElementById('ota-content');
            if (otaContent) {
                otaContent.innerHTML = '<p style="color:red;">Failed to load OTA firmware information.</p>';
            }
        });
}

function loadOTAFilesystemContent() {
    fetch('/ota-fs.html')
        .then(r => r.text())
        .then(html => {
            const otaContentFS = document.getElementById('ota-content-fs');
            if (otaContentFS) {
                otaContentFS.innerHTML = html;
                otaFilesystemLoaded = true;
            }
        })
        .catch(e => {
            const otaContentFS = document.getElementById('ota-content-fs');
            if (otaContentFS) {
                otaContentFS.innerHTML = '<p style="color:red;">Failed to load OTA filesystem information.</p>';
            }
        });
}

function loadAboutContent() {
    fetch('/about.html')
        .then(r => r.text())
        .then(html => {
            const aboutContent = document.getElementById('about-content');
            if (aboutContent) {
                aboutContent.innerHTML = html;
                aboutLoaded = true;
            }
        })
        .catch(e => {
            const aboutContent = document.getElementById('about-content');
            if (aboutContent) {
                aboutContent.innerHTML = '<p style="color:red;">Failed to load About information.</p>';
            }
        });
}

function pollSerial(port) {
    if ((port === 0 && !document.getElementById('tab-serial0').classList.contains('active')) ||
        (port === 1 && !document.getElementById('tab-serial1').classList.contains('active'))) {
        if (port === 0) polling0 = false;
        if (port === 1) polling1 = false;
        return;
    }

    fetch('/serial' + port + '/poll')
        .then(r => r.text())
        .then(d => {
            if (d.length > 0) {
                const o = document.getElementById('output' + port);
                o.textContent += d;
                o.scrollTop = o.scrollHeight;
            }
            setTimeout(() => pollSerial(port), 500);
        })
        .catch(e => setTimeout(() => pollSerial(port), 1000));
}

function sendCommand(port) {
    const i = document.getElementById('input' + port);
    let c = i.value;
    if (!c) return;

    // Check if auto newline is enabled and add \r\n
    const autoNewline = (port === 0) ? autoNewline0 : autoNewline1;
    if (autoNewline && !c.endsWith('\r\n')) {
        c += '\r\n';
    }

    // Local echo
    const localEcho = (port === 0) ? localEcho0 : localEcho1;
    const passwordMode = (port === 0) ? passwordMode0 : passwordMode1;

    if (localEcho) {
        const o = document.getElementById('output' + port);
        const displayText = passwordMode ? '*'.repeat(c.length) : c;
        o.textContent += '$web$' + displayText + '\n';
        o.scrollTop = o.scrollHeight;
    }

    fetch('/serial' + port + '/send', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'data=' + encodeURIComponent(c)
    })
    .then(r => r.text())
    .then(d => {
        i.value = '';
    })
    .catch(e => console.error('Error:', e));
}

function clearOutput(port) {
    document.getElementById('output' + port).textContent = '';
}

function sendSpecial(port, type) {
    let data = '';
    let displayName = '';

    switch(type) {
        case 'ESC':
            data = '\x1b';
            displayName = '\\e';
            break;
        case 'CTRL+BACKTICK':
            data = '\x60';
            displayName = '`';
            break;
        case 'TAB':
            data = '\t';
            displayName = '\\t';
            break;
        case 'ENTER':
            data = '\r\n';
            displayName = '\\r\\n';
            break;
        case 'CTRL+C':
            data = '\x03';
            displayName = '^C';
            break;
        case 'CTRL+Z':
            data = '\x1a';
            displayName = '^Z';
            break;
        case 'CTRL+D':
            data = '\x04';
            displayName = '^D';
            break;
        case 'CTRL+A':
            data = '\x01';
            displayName = '^A';
            break;
        case 'CTRL+E':
            data = '\x05';
            displayName = '^E';
            break;
        case 'BACKSPACE':
            data = '\x08';
            displayName = '^H';
            break;
        default: return;
    }

    // Local echo for special characters
    const localEcho = (port === 0) ? localEcho0 : localEcho1;
    if (localEcho) {
        const o = document.getElementById('output' + port);
        if (o) {
            o.textContent += '$web$' + displayName + '\n';
            o.scrollTop = o.scrollHeight;
        }
    }

    fetch('/serial' + port + '/send', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'data=' + encodeURIComponent(data)
    })
    .then(r => r.text())
    .then(d => {})
    .catch(e => console.error('Error:', e));
}

// Password validation
document.addEventListener('DOMContentLoaded', function() {
    const form = document.querySelector('form');
    if (!form) {
        console.error('Form element not found');
        return;
    }
    
    form.addEventListener('submit', function(e) {
        const wifiPwd = document.getElementById('wifi_password');
        const wifiPwdHasValueInput = document.getElementById('wifi_password_has_value');
        const mqttPwd = document.getElementById('mqtt_password');
        const mqttPwdHasValueInput = document.getElementById('mqtt_password_has_value');

        if (!wifiPwd || !wifiPwdHasValueInput || !mqttPwd || !mqttPwdHasValueInput) {
            console.error('Required password elements not found');
            return;
        }

        const wifiPwdHasValue = wifiPwdHasValueInput.value === '1';
        const mqttPwdHasValue = mqttPwdHasValueInput.value === '1';

        if (wifiPwdHasValue && wifiPwd.value === '********') {
            e.preventDefault();
            alert('Invalid password: Please enter a new password or leave empty to keep current.');
            return false;
        }

        if (mqttPwdHasValue && mqttPwd.value === '********') {
            e.preventDefault();
            alert('Invalid password: Please enter a new password or leave empty to keep current.');
            return false;
        }

        if (wifiPwd.value === '********') wifiPwd.value = '';
        if (mqttPwd.value === '********') mqttPwd.value = '';
    });
});

function togglePassword(port) {
    const input = document.getElementById('input' + port);
    const btn = document.getElementById('passwordBtn' + port);

    if (port === 0) {
        passwordMode0 = !passwordMode0;
        input.type = passwordMode0 ? 'password' : 'text';

        // Update button appearance
        if (passwordMode0) {
            btn.classList.add('active');
            btn.textContent = 'Password ON';
        } else {
            btn.classList.remove('active');
            btn.textContent = 'Password';
        }
    } else if (port === 1) {
        passwordMode1 = !passwordMode1;
        input.type = passwordMode1 ? 'password' : 'text';

        // Update button appearance
        if (passwordMode1) {
            btn.classList.add('active');
            btn.textContent = 'Password ON';
        } else {
            btn.classList.remove('active');
            btn.textContent = 'Password';
        }
    }

    input.focus();
    input.setSelectionRange(input.value.length, input.value.length);
}

function toggleAutoNewline(port) {
    const btn = document.getElementById('autoNewlineBtn' + port);

    if (!btn) {
        console.error('Auto Newline button not found for port:', port);
        return;
    }

    if (port === 0) {
        autoNewline0 = !autoNewline0;

        // Update button appearance
        if (autoNewline0) {
            btn.classList.add('active');
            btn.textContent = 'Auto NL ON';
        } else {
            btn.classList.remove('active');
            btn.textContent = 'Auto NL';
        }
    } else if (port === 1) {
        autoNewline1 = !autoNewline1;

        // Update button appearance
        if (autoNewline1) {
            btn.classList.add('active');
            btn.textContent = 'Auto NL ON';
        } else {
            btn.classList.remove('active');
            btn.textContent = 'Auto NL';
        }
    }
}

function toggleLocalEcho(port) {
    const btn = document.getElementById('localEchoBtn' + port);

    if (!btn) {
        console.error('Button not found for port:', port);
        return;
    }

    if (port === 0) {
        localEcho0 = !localEcho0;

        // Update button appearance
        if (localEcho0) {
            btn.classList.add('active');
            btn.textContent = 'Echo ON';
        } else {
            btn.classList.remove('active');
            btn.textContent = 'Local Echo';
        }
    } else if (port === 1) {
        localEcho1 = !localEcho1;

        // Update button appearance
        if (localEcho1) {
            btn.classList.add('active');
            btn.textContent = 'Echo ON';
        } else {
            btn.classList.remove('active');
            btn.textContent = 'Local Echo';
        }
    }
}

// OTA Upload Functions
let uploadInProgress = false;
let currentUploadType = '';

function handleAutoUpload(event, type) {
    console.log('handleAutoUpload called with type:', type);
    const fileInput = document.getElementById(type + '-file');

    if (!fileInput || !fileInput.files || !fileInput.files[0]) {
        console.error('No file selected');
        return;
    }

    const file = fileInput.files[0];
    console.log('File selected:', file.name, 'size:', file.size);

    // Store file reference
    window[type + 'File'] = file;
    currentUploadType = type;

    // Start upload immediately
    startUpload(type);
}

function handleFileSelect(event, type) {
    console.log('handleFileSelect called with type:', type);
    const fileInput = document.getElementById(type + '-file');
    const submitBtn = document.getElementById(type + '-submit-btn');
    const filePathInput = document.getElementById(type + '-file-path');

    // Validate required elements exist
    if (!fileInput || !submitBtn) {
        console.error('Required UI elements not found for type:', type);
        return;
    }

    if (fileInput.files && fileInput.files[0]) {
        const file = fileInput.files[0];
        console.log('File selected:', file.name, 'size:', file.size);

        // Store file reference
        window[type + 'File'] = file;
        currentUploadType = type;

        // Update file path input to show selected file
        if (filePathInput) {
            filePathInput.value = file.name;
        }

        // Show submit button
        submitBtn.style.display = 'block';
        console.log('Submit button shown for type:', type);
    } else {
        console.error('No file found in fileInput.files');
    }
}

function selectFile(type) {
    console.log('selectFile called with type:', type);
    const fileInput = document.getElementById(type + '-file');
    if (fileInput) {
        fileInput.click();
        console.log('File input clicked');
    } else {
        console.error('File input element not found:', type + '-file');
    }
}

// Drag and drop functionality removed - using simple file input instead

async function startUpload(type) {
    console.log('startUpload called with type:', type);

    if (uploadInProgress) {
        console.log('Upload already in progress, aborting');
        return;
    }

    const file = window[type + 'File'];
    if (!file) {
        console.error('No file selected for type:', type);
        alert('No file selected. Please select a file first.');
        return;
    }

    console.log('Starting upload for file:', file.name, 'size:', file.size);

    uploadInProgress = true;
    const isFirmware = type === 'firmware';
    const endpoint = isFirmware ? '/ota/firmware/upload' : '/ota/filesystem/upload';

    console.log('Upload endpoint:', endpoint);

    // Get UI elements - use fallback approach
    const progressDiv = document.getElementById(type + '-progress');
    const progressBar = document.getElementById(type + '-progress-bar');
    const progressText = document.getElementById(type + '-progress-text');
    const statusDiv = document.getElementById(type + '-status');
    const submitBtn = document.getElementById(type + '-submit-btn');

    // Log which elements were found
    console.log('Found elements:', {
        progressDiv: !!progressDiv,
        progressBar: !!progressBar,
        progressText: !!progressText,
        statusDiv: !!statusDiv,
        submitBtn: !!submitBtn
    });

    // Update UI safely
    if (progressDiv) progressDiv.style.display = 'block';
    if (progressBar) progressBar.style.width = '0%';
    if (progressText) progressText.textContent = '0%';
    if (statusDiv) {
        statusDiv.className = 'status-message info';
        statusDiv.textContent = 'Calculating SHA256...';
    }
    if (submitBtn) submitBtn.disabled = true;

    try {
        // Calculate SHA256 hash
        console.log('Calculating SHA256 hash...');
        const hash = await calculateSHA256(file);

        if (hash) {
            console.log('SHA256 hash:', hash);
        } else {
            console.log('Uploading without hash verification (Web Crypto API not available)');
        }

        // Update status
        if (statusDiv) {
            statusDiv.className = 'status-message info';
            statusDiv.textContent = 'Uploading file...';
        }

        // Create form data
        const formData = new FormData();
        formData.append('file', file);
        formData.append('hash', hash);

        console.log('FormData created, starting XHR upload...');

        // Create XMLHttpRequest for progress tracking
        const xhr = new XMLHttpRequest();

        // Track upload progress
        xhr.upload.addEventListener('progress', (e) => {
            if (e.lengthComputable) {
                const percentComplete = Math.round((e.loaded / e.total) * 100);
                console.log('Upload progress:', percentComplete + '%');
                if (progressBar) progressBar.style.width = percentComplete + '%';
                if (progressText) progressText.textContent = percentComplete + '%';
            }
        });

        // Handle completion
        xhr.addEventListener('load', () => {
            console.log('Upload completed. Status:', xhr.status, 'Response:', xhr.responseText);

            if (xhr.status === 200) {
                if (statusDiv) statusDiv.className = 'status-message success';

                if (isFirmware) {
                    if (statusDiv) statusDiv.textContent = 'Firmware uploaded successfully! Device will restart...';
                    // Disable all upload controls
                    const firmwareSubmitBtn = document.getElementById('firmware-submit-btn');
                    const filesystemSubmitBtn = document.getElementById('filesystem-submit-btn');
                    if (firmwareSubmitBtn) firmwareSubmitBtn.disabled = true;
                    if (filesystemSubmitBtn) filesystemSubmitBtn.disabled = true;

                    // Refresh page after delay
                    setTimeout(() => {
                        window.location.reload();
                    }, 10000);
                } else {
                    if (statusDiv) statusDiv.textContent = 'Filesystem uploaded successfully!';
                    setTimeout(() => {
                        if (statusDiv) {
                            statusDiv.className = 'status-message';
                            statusDiv.textContent = '';
                        }
                    }, 3000);
                }
            } else {
                console.error('Upload failed with status:', xhr.status, xhr.statusText);
                if (statusDiv) {
                    statusDiv.className = 'status-message error';
                    statusDiv.textContent = 'Upload failed: ' + xhr.statusText + ' - ' + xhr.responseText;
                }
            }

            uploadInProgress = false;
            if (submitBtn) submitBtn.disabled = false;
        });

        // Handle errors
        xhr.addEventListener('error', () => {
            console.error('XHR error occurred');
            if (statusDiv) {
                statusDiv.className = 'status-message error';
                statusDiv.textContent = 'Upload failed. Please try again.';
            }
            uploadInProgress = false;
            if (submitBtn) submitBtn.disabled = false;
        });

        // Send request
        console.log('Opening POST request to:', endpoint);
        xhr.open('POST', endpoint, true);
        console.log('Sending FormData...');
        xhr.send(formData);

    } catch (error) {
        console.error('Error during upload:', error);
        if (statusDiv) {
            statusDiv.className = 'status-message error';
            statusDiv.textContent = 'Error: ' + error.message;
        } else {
            alert('Upload error: ' + error.message);
        }
        uploadInProgress = false;
        if (submitBtn) submitBtn.disabled = false;
    }
}

async function calculateSHA256(file) {
    // Check if Web Crypto API is available
    if (!window.crypto || !window.crypto.subtle) {
        console.warn('Web Crypto API not available. Hash verification will be skipped.');
        console.warn('This is normal when accessing via HTTP. File will be uploaded without hash verification.');
        return '';  // Return empty string to skip hash verification
    }

    return new Promise((resolve, reject) => {
        const reader = new FileReader();

        reader.onload = async (event) => {
            try {
                // Use Web Crypto API for SHA256
                const buffer = event.target.result;
                const hashBuffer = await crypto.subtle.digest('SHA-256', buffer);

                // Convert to hex string
                const hashArray = Array.from(new Uint8Array(hashBuffer));
                const hashHex = hashArray.map(b => b.toString(16).padStart(2, '0')).join('');

                resolve(hashHex);
            } catch (error) {
                console.error('Error calculating SHA256:', error);
                console.warn('Hash calculation failed. Uploading without hash verification.');
                resolve('');  // Return empty string instead of rejecting
            }
        };

        reader.onerror = () => {
            console.error('Failed to read file for hashing');
            resolve('');  // Return empty string instead of rejecting
        };

        reader.readAsArrayBuffer(file);
    });
}