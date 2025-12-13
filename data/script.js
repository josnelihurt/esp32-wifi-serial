let lastPos0 = 0;
let lastPos1 = 0;
let polling0 = false;
let polling1 = false;
let otaLoaded = false;
let aboutLoaded = false;

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
    switch(type) {
        case 'ESC': data = '\x1b'; break;
        case 'CTRL+BACKTICK': data = '\x60'; break;
        case 'TAB': data = '\t'; break;
        case 'ENTER': data = '\r\n'; break;
        case 'CTRL+C': data = '\x03'; break;
        case 'CTRL+Z': data = '\x1a'; break;
        case 'CTRL+D': data = '\x04'; break;
        case 'CTRL+A': data = '\x01'; break;
        case 'CTRL+E': data = '\x05'; break;
        case 'BACKSPACE': data = '\x08'; break;
        default: return;
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
