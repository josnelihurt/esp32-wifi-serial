let lastPos0 = 0;
let lastPos1 = 0;
let polling0 = false;
let polling1 = false;
let otaLoaded = false;
let aboutLoaded = false;
let localEcho0 = false;
let localEcho1 = false;
let passwordMode0 = false;
let passwordMode1 = false;

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

    if (index === 3 && !otaLoaded) {
        loadOTAContent();
    }

    if (index === 4 && !aboutLoaded) {
        loadAboutContent();
    }
}

function loadOTAContent() {
    fetch('/ota.html')
        .then(r => r.text())
        .then(html => {
            document.getElementById('ota-content').innerHTML = html;
            otaLoaded = true;
        })
        .catch(e => {
            document.getElementById('ota-content').innerHTML = '<p style="color:red;">Failed to load OTA information.</p>';
        });
}

function loadAboutContent() {
    fetch('/about.html')
        .then(r => r.text())
        .then(html => {
            document.getElementById('about-content').innerHTML = html;
            aboutLoaded = true;
        })
        .catch(e => {
            document.getElementById('about-content').innerHTML = '<p style="color:red;">Failed to load About information.</p>';
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
            setTimeout(() => pollSerial(port), 100);
        })
        .catch(e => setTimeout(() => pollSerial(port), 1000));
}

function sendCommand(port) {
    const i = document.getElementById('input' + port);
    const c = i.value;
    if (!c) return;

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
        o.textContent += '$web$' + displayName + '\n';
        o.scrollTop = o.scrollHeight;
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
    document.querySelector('form').addEventListener('submit', function(e) {
        const wifiPwd = document.getElementById('wifi_password');
        const wifiPwdHasValue = document.getElementById('wifi_password_has_value').value === '1';
        const mqttPwd = document.getElementById('mqtt_password');
        const mqttPwdHasValue = document.getElementById('mqtt_password_has_value').value === '1';

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

function toggleLocalEcho(port) {
    const btn = document.getElementById('localEchoBtn' + port);

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