#include "web_config_server.h"
#include "domain/config/preferences_storage.h"
#include "domain/serial/serial_log.h"
#include <Arduino.h>
#include <ArduinoLog.h>
#include <Preferences.h>
namespace jrb::wifi_serial {

WebConfigServer::WebConfigServer(PreferencesStorage &storage,
                                 SerialLog &serial0Log, SerialLog &serial1Log,
                                 SerialSendCallback sendCallback)
    : preferencesStorage(storage), serial0Log(serial0Log),
      serial1Log(serial1Log), sendCallback(sendCallback), server(nullptr),
      apMode(false) {
  Log.traceln(__PRETTY_FUNCTION__);
}

WebConfigServer::~WebConfigServer() {
  if (server) {
    server->stop();
    delete server;
  }
}

void WebConfigServer::loop() {
  if (server) {
    server->handleClient();
  }
}

void WebConfigServer::setWiFiConfig(const String &ssid, const String &password,
                                    const String &deviceName,
                                    const String &mqttBroker, int mqttPort,
                                    const String &mqttUser,
                                    const String &mqttPassword) {}

void WebConfigServer::setAPMode(bool apMode) {
  Log.infoln(__PRETTY_FUNCTION__, "apMode: %s", apMode ? "true" : "false");
  this->apMode = apMode;
}

void WebConfigServer::setAPIP(const IPAddress &ip) {
  Log.infoln(__PRETTY_FUNCTION__, "ip: %s", ip.toString().c_str());
  this->apIP = ip;
}

void WebConfigServer::begin() {
  if (server) {
    server->stop();
    delete server;
    server = nullptr;
  }
  Log.infoln(__PRETTY_FUNCTION__, "Creating web server");
  server = new WebServer(80);

  server->on("/", HTTP_GET, [this]() {
    Log.traceln("%s: %s", __PRETTY_FUNCTION__, "Handling / request");
    server->send(200, "text/html", this->getConfigHTML());
  });

  server->on("/save", HTTP_POST, [this]() {
    Log.traceln("%s: %s", __PRETTY_FUNCTION__, "Handling /save request");
    if (server->hasArg("speed0")) {
      preferencesStorage.baudRateTty1 = server->arg("speed0").toInt();
    }
    if (server->hasArg("ssid")) {
      preferencesStorage.ssid = server->arg("ssid");
    }
    if (server->hasArg("password")) {
      String newPassword = server->arg("password");
      if (newPassword.length() > 0 && newPassword != "********") {
        preferencesStorage.password = newPassword;
      }
    }
    if (server->hasArg("device")) {
      preferencesStorage.deviceName = server->arg("device");
      preferencesStorage.topicTty0Rx =
          "wifi_serial/" + preferencesStorage.deviceName + "/ttyS0/rx";
      preferencesStorage.topicTty0Tx =
          "wifi_serial/" + preferencesStorage.deviceName + "/ttyS0/tx";
      preferencesStorage.topicTty1Rx =
          "wifi_serial/" + preferencesStorage.deviceName + "/ttyS1/rx";
      preferencesStorage.topicTty1Tx =
          "wifi_serial/" + preferencesStorage.deviceName + "/ttyS1/tx";
    }
    if (server->hasArg("broker")) {
      preferencesStorage.mqttBroker = server->arg("broker");
    }
    if (server->hasArg("port")) {
      preferencesStorage.mqttPort = server->arg("port").toInt();
    }
    if (server->hasArg("user")) {
      preferencesStorage.mqttUser = server->arg("user");
    }
    if (server->hasArg("mqttpass")) {
      String newMqttPassword = server->arg("mqttpass");
      if (newMqttPassword.length() > 0 && newMqttPassword != "********") {
        preferencesStorage.mqttPassword = newMqttPassword;
      }
    }
    preferencesStorage.save();
    server->send(200, "text/plain", "Configuration saved! Restarting...");
    delay(1000);
    ESP.restart();
  });

  server->on("/reset", HTTP_POST, [this]() {
    Log.traceln("%s: %s", __PRETTY_FUNCTION__, "Handling /reset request");
    server->send(200, "text/plain", "Device resetting...");
    delay(500);
    ESP.restart();
  });

  server->on("/serial0/poll", HTTP_GET, [this]() {
    Log.traceln("%s: %s", __PRETTY_FUNCTION__, "Handling /serial0/poll request");
    static int lastPos0 = 0;
    unsigned long startTime = millis();

    while (millis() - startTime < 3000) {
      String newData = serial0Log.getNewData(lastPos0);
      if (newData.length() > 0) {
        server->send(200, "text/plain", newData);
        return;
      }
      delay(100);
      server->handleClient();
    }

    server->send(200, "text/plain", "");
  });

  server->on("/serial1/poll", HTTP_GET, [this]() {
    Log.traceln("%s: %s", __PRETTY_FUNCTION__, "Handling /serial1/poll request");
    static int lastPos1 = 0;
    unsigned long startTime = millis();

    while (millis() - startTime < 3000) {
      String newData = serial1Log.getNewData(lastPos1);
      if (newData.length() > 0) {
        server->send(200, "text/plain", newData);
        return;
      }
      delay(100);
      server->handleClient();
    }

    server->send(200, "text/plain", "");
  });

  server->on("/serial0/send", HTTP_POST, [this]() {
    Log.traceln("%s: %s", __PRETTY_FUNCTION__, "Handling /serial0/send request");
    if (!server->hasArg("data")) {
      server->send(400, "text/plain", "Missing data");
      return;
    }

    String data = server->arg("data");
    if (sendCallback) {
      sendCallback(0, data);
    }
    server->send(200, "text/plain", "OK");
  });

  server->on("/serial1/send", HTTP_POST, [this]() {
    Log.traceln("%s: %s", __PRETTY_FUNCTION__, "Handling /serial1/send request");
    if (!server->hasArg("data")) {
      server->send(400, "text/plain", "Missing data");
      return;
    }

    String data = server->arg("data");
    if (sendCallback) {
      sendCallback(1, data);
    }
    server->send(200, "text/plain", "OK");
  });

  server->begin();
}

String WebConfigServer::getConfigHTML() {
  return String(R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32-C3 Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body{font-family:Arial,sans-serif;margin:0;padding:20px;background:#FFFFFF;color:#333333;}
        h2,h3{color:#FFC107;margin-top:0;}
        a{color:#FFC107;text-decoration:none;}
        a:hover{color:#FFA000;text-decoration:underline;}
        input{width:100%;padding:10px;margin:5px 0;box-sizing:border-box;border:2px solid #666666;border-radius:4px;color:#000000;background:#FFFFFF;font-size:14px;}
        input:focus{border-color:#FFC107;outline:none;border-width:2px;box-shadow:0 0 5px rgba(255,193,7,0.3);}
        button{background:#FFC107;color:#333333;padding:15px;width:100%;border:none;cursor:pointer;border-radius:4px;font-weight:bold;}
        button:hover{background:#FFA000;color:#FFFFFF;}
        .tabs{border-bottom:2px solid #CCCCCC;margin-bottom:20px;}
        .tab{display:inline-block;padding:10px 20px;cursor:pointer;border:1px solid #CCCCCC;background:#FFFFFF;color:#FFC107;border-bottom:none;margin-right:5px;border-radius:4px 4px 0 0;}
        .tab:hover{background:#FFF9C4;}
        .tab.active{background:#FFC107;color:#333333;border-color:#FFC107;font-weight:bold;}
        .tab-content{display:none;padding:20px;background:#FFFFFF;}
        .tab-content.active{display:block;}
        .serial-output{width:100%;height:400px;padding:10px;box-sizing:border-box;border:1px solid #CCCCCC;font-family:monospace;background:#FFFFFF;color:#333333;overflow-y:auto;white-space:pre-wrap;word-wrap:break-word;border-radius:4px;}
        .serial-input{width:calc(100% - 100px);padding:10px;margin-right:10px;box-sizing:border-box;border:2px solid #666666;border-radius:4px;color:#000000;background:#FFFFFF;font-size:14px;}
        .serial-input:focus{border-color:#FFC107;outline:none;border-width:2px;box-shadow:0 0 5px rgba(255,193,7,0.3);}
        .send-btn{width:90px;padding:10px;background:#FFC107;color:#333333;border:none;cursor:pointer;border-radius:4px;font-weight:bold;}
        .send-btn:hover{background:#FFA000;color:#FFFFFF;}
        .input-group{display:flex;margin-top:10px;}
        label{display:block;margin-top:10px;font-weight:bold;color:#333333;}
        form{margin-bottom:20px;}
        .reset-btn{background:#f44336;color:#FFFFFF;padding:15px;width:100%;border:none;cursor:pointer;border-radius:4px;font-weight:bold;margin-top:10px;}
        .reset-btn:hover{background:#d32f2f;}
        .clear-btn{background:#2196F3;color:#FFFFFF;padding:8px 12px;border:none;cursor:pointer;border-radius:4px;font-weight:bold;}
        .clear-btn:hover{background:#1976D2;}
        .special-btn{background:#4CAF50;color:#FFFFFF;padding:8px 12px;border:none;cursor:pointer;border-radius:4px;font-weight:bold;}
        .special-btn:hover{background:#388E3C;}
        .button-table{width:100%;border-collapse:collapse;margin-bottom:10px;}
        .button-table td{padding:2px;}
    </style>
</head>
<body>
    <h2>ESP32-C3 Configuration</h2>
    <div class="tabs">
        <div class="tab active" onclick="switchTab(0)">Configuration</div>
        <div class="tab" onclick="switchTab(1)">ttyS0 (USB)</div>
        <div class="tab" onclick="switchTab(2)">ttyS1 (UART)</div>
    </div>
    <div id="tab-config" class="tab-content active">
        <h3>Configuration</h3>
        <form action="/save" method="POST">
            <label>WiFi SSID:</label>
            <input type="text" name="ssid" value=")HTML") +
         escapeHTML(preferencesStorage.ssid) + String(R"HTML(" required>
            <label>WiFi Password:</label>
            <input type="password" name="password" id="wifi_password" value=")HTML") +
         (preferencesStorage.password.length() > 0 ? String("********")
                                                   : String("")) +
         String(R"HTML(" placeholder="Leave empty to keep current">
            <input type="hidden" id="wifi_password_has_value" value=")HTML") +
         (preferencesStorage.password.length() > 0 ? String("1")
                                                   : String("0")) +
         String(R"HTML(">
            <label>TTYS1 (UART) Speed (baud):</label>
            <input type="number" name="speed0" value=")HTML") +
         String(preferencesStorage.baudRateTty1) + String(R"HTML(">
            <label>Device Name:</label>
            <input type="text" name="device" value=")HTML") +
         escapeHTML(preferencesStorage.deviceName.length() > 0
                        ? preferencesStorage.deviceName
                        : "esp32c3") +
         String(R"HTML(">
            <label>MQTT Broker:</label>
            <input type="text" name="broker" value=")HTML") +
         escapeHTML(preferencesStorage.mqttBroker) + String(R"HTML(">
            <label>MQTT Topics ttyS0:</label>
            <label style="margin-top:5px;font-size:12px;color:#666666;">RX (ESP32 -> MQTT):</label>
            <div style="padding:10px;background:#f5f5f5;border:2px solid #666666;border-radius:4px;color:#000000;font-family:monospace;margin:5px 0;">)HTML") +
         escapeHTML(preferencesStorage.topicTty0Rx) + String(R"HTML(</div>
            <label style="margin-top:5px;font-size:12px;color:#666666;">TX (MQTT -> ESP32):</label>
            <div style="padding:10px;background:#f5f5f5;border:2px solid #666666;border-radius:4px;color:#000000;font-family:monospace;margin:5px 0;">)HTML") +
         escapeHTML(preferencesStorage.topicTty0Tx) + String(R"HTML(</div>
            <label>MQTT Topics ttyS1:</label>
            <label style="margin-top:5px;font-size:12px;color:#666666;">RX (ESP32 -> MQTT):</label>
            <div style="padding:10px;background:#f5f5f5;border:2px solid #666666;border-radius:4px;color:#000000;font-family:monospace;margin:5px 0;">)HTML") +
         escapeHTML(preferencesStorage.topicTty1Rx) + String(R"HTML(</div>
            <label style="margin-top:5px;font-size:12px;color:#666666;">TX (MQTT -> ESP32):</label>
            <div style="padding:10px;background:#f5f5f5;border:2px solid #666666;border-radius:4px;color:#000000;font-family:monospace;margin:5px 0;">)HTML") +
         escapeHTML(preferencesStorage.topicTty1Tx) + String(R"HTML(</div>
            <label>MQTT Port:</label>
            <input type="number" name="port" value=")HTML") +
         String(preferencesStorage.mqttPort > 0 ? preferencesStorage.mqttPort
                                                : 1883) +
         String(R"HTML(">
            <label>MQTT User (optional):</label>
            <input type="text" name="user" value=")HTML") +
         escapeHTML(preferencesStorage.mqttUser) + String(R"HTML(">
            <label>MQTT Password (optional):</label>
            <input type="password" name="mqttpass" id="mqtt_password" value=")HTML") +
         (preferencesStorage.mqttPassword.length() > 0 ? String("********")
                                                       : String("")) +
         String(R"HTML(" placeholder="Leave empty to keep current">
            <input type="hidden" id="mqtt_password_has_value" value=")HTML") +
         (preferencesStorage.mqttPassword.length() > 0 ? String("1")
                                                       : String("0")) +
         String(R"HTML(">
            <button type="submit">Save Configuration</button>
        </form>
        <button class="reset-btn" onclick="if(confirm('Are you sure you want to reset the device? This will restart the ESP32.')){fetch('/reset',{method:'POST'}).then(()=>{alert('Device resetting...');});}">Reset Device</button>
    </div>
    <div id="tab-serial0" class="tab-content">
        <h3>Serial Port ttyS0 (USB)</h3>
        <div id="output0" class="serial-output"></div>
        <table class="button-table">
            <tr>
                <td><button class="special-btn" onclick="sendSpecial(0, 'ESC')">ESC \e</button></td>
                <td><button class="special-btn" onclick="sendSpecial(0, 'TAB')">TAB \t</button></td>
                <td><button class="special-btn" onclick="sendSpecial(0, 'ENTER')">ENTER \r\n</button></td>
                <td><button class="special-btn" onclick="sendSpecial(0, 'CTRL+C')">Ctrl+C ^C</button></td>
                <td><button class="special-btn" onclick="sendSpecial(0, 'CTRL+Z')">Ctrl+Z ^Z</button></td>
                <td><button class="special-btn" onclick="sendSpecial(0, 'CTRL+D')">Ctrl+D ^D</button></td>
                <td><button class="special-btn" onclick="sendSpecial(0, 'CTRL+A')">Ctrl+A ^A</button></td>
                <td><button class="special-btn" onclick="sendSpecial(0, 'CTRL+E')">Ctrl+E ^E</button></td>
                <td><button class="special-btn" onclick="sendSpecial(0, 'BACKSPACE')">Backspace ^H</button></td>
                <td><button class="clear-btn" onclick="clearOutput(0)">Clear</button></td>
            </tr>
        </table>
        <div class="input-group">
            <input type="text" id="input0" class="serial-input" placeholder="Enter command..." onkeypress="if(event.key==='Enter')sendCommand(0)">
            <button class="send-btn" onclick="sendCommand(0)">Send</button>
        </div>
    </div>
    <div id="tab-serial1" class="tab-content">
        <h3>Serial Port ttyS1 (UART)</h3>
        <div id="output1" class="serial-output"></div>
        <table class="button-table">
            <tr>
                <td><button class="special-btn" onclick="sendSpecial(1, 'ESC')">ESC \e</button></td>
                <td><button class="special-btn" onclick="sendSpecial(1, 'TAB')">TAB \t</button></td>
                <td><button class="special-btn" onclick="sendSpecial(1, 'ENTER')">ENTER \r\n</button></td>
                <td><button class="special-btn" onclick="sendSpecial(1, 'CTRL+C')">Ctrl+C ^C</button></td>
                <td><button class="special-btn" onclick="sendSpecial(1, 'CTRL+Z')">Ctrl+Z ^Z</button></td>
                <td><button class="special-btn" onclick="sendSpecial(1, 'CTRL+D')">Ctrl+D ^D</button></td>
                <td><button class="special-btn" onclick="sendSpecial(1, 'CTRL+A')">Ctrl+A ^A</button></td>
                <td><button class="special-btn" onclick="sendSpecial(1, 'CTRL+E')">Ctrl+E ^E</button></td>
                <td><button class="special-btn" onclick="sendSpecial(1, 'BACKSPACE')">Backspace ^H</button></td>
                <td><button class="clear-btn" onclick="clearOutput(1)">Clear</button></td>
            </tr>
        </table>
        <div class="input-group">
            <input type="text" id="input1" class="serial-input" placeholder="Enter command..." onkeypress="if(event.key==='Enter')sendCommand(1)">
            <button class="send-btn" onclick="sendCommand(1)">Send</button>
        </div>
    </div>
    <script>
        let lastPos0=0;let lastPos1=0;let polling0=false;let polling1=false;
        function switchTab(index){
            document.querySelectorAll('.tab').forEach((t,i)=>{t.classList.toggle('active',i===index);});
            document.querySelectorAll('.tab-content').forEach((c,i)=>{c.classList.toggle('active',i===index);});
            if(index===1&&!polling0){polling0=true;pollSerial(0);}else if(index!==1){polling0=false;}
            if(index===2&&!polling1){polling1=true;pollSerial(1);}else if(index!==2){polling1=false;}
        }
        function pollSerial(port){
            if((port===0&&!document.getElementById('tab-serial0').classList.contains('active'))||
               (port===1&&!document.getElementById('tab-serial1').classList.contains('active'))){
                if(port===0)polling0=false;if(port===1)polling1=false;return;}
            fetch('/serial'+port+'/poll').then(r=>r.text()).then(d=>{
                if(d.length>0){const o=document.getElementById('output'+port);o.textContent+=d;o.scrollTop=o.scrollHeight;}
                setTimeout(()=>pollSerial(port),100);}).catch(e=>setTimeout(()=>pollSerial(port),1000));}
         function sendCommand(port){
             const i=document.getElementById('input'+port);const c=i.value;if(!c)return;
             fetch('/serial'+port+'/send',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'data='+encodeURIComponent(c)})
             .then(r=>r.text()).then(d=>{i.value='';}).catch(e=>console.error('Error:',e));}
         function clearOutput(port){
             document.getElementById('output'+port).textContent='';}
         function sendSpecial(port, type){
             let data='';
             switch(type){
                 case 'ESC': data='\x1b'; break;
                 case 'TAB': data='\t'; break;
                 case 'ENTER': data='\r\n'; break;
                 case 'CTRL+C': data='\x03'; break;
                 case 'CTRL+Z': data='\x1a'; break;
                 case 'CTRL+D': data='\x04'; break;
                 case 'CTRL+A': data='\x01'; break;
                 case 'CTRL+E': data='\x05'; break;
                 case 'BACKSPACE': data='\x08'; break;
                 default: return;
             }
             fetch('/serial'+port+'/send',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'data='+encodeURIComponent(data)})
             .then(r=>r.text()).then(d=>{}).catch(e=>console.error('Error:',e));}
        document.querySelector('form').addEventListener('submit',function(e){
            const wifiPwd=document.getElementById('wifi_password');
            const wifiPwdHasValue=document.getElementById('wifi_password_has_value').value==='1';
            const mqttPwd=document.getElementById('mqtt_password');
            const mqttPwdHasValue=document.getElementById('mqtt_password_has_value').value==='1';
            if(wifiPwdHasValue&&wifiPwd.value==='********'){
                e.preventDefault();
                alert('Invalid password: Please enter a new password or leave empty to keep current.');
                return false;
            }
            if(mqttPwdHasValue&&mqttPwd.value==='********'){
                e.preventDefault();
                alert('Invalid password: Please enter a new password or leave empty to keep current.');
                return false;
            }
            if(wifiPwd.value==='********')wifiPwd.value='';
            if(mqttPwd.value==='********')mqttPwd.value='';
        });
    </script>
</body>
</html>
)HTML");
}

String WebConfigServer::escapeHTML(const String &str) {
  String escaped = str;
  escaped.replace("&", "&amp;");
  escaped.replace("<", "&lt;");
  escaped.replace(">", "&gt;");
  escaped.replace("\"", "&quot;");
  escaped.replace("'", "&#39;");
  return escaped;
}

} // namespace jrb::wifi_serial
