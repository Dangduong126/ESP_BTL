#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>


#define STM_RX 16
#define STM_TX 17

const char *ssid = "ESP32_BTL";
const char *password = "12345678";

WebServer server(80);

float temp = 0;
float hum = 0;
int gasValue = 0;

String receivedData = "";
String ledStatus = "OFF";


void parseData()
{
    int tIndex = receivedData.indexOf("T:");
    int hIndex = receivedData.indexOf(",H:");
    int gIndex = receivedData.indexOf(",G:");


    if(tIndex != -1 && hIndex != -1 && gIndex != -1)
    {
        temp = receivedData.substring(tIndex + 2,hIndex).toFloat();
        hum = receivedData.substring(hIndex + 3,gIndex).toFloat();
        gasValue = receivedData.substring(gIndex + 3).toInt();
    }
}

void readSTM(){
    if(Serial2.available())
    {
        receivedData =  Serial2.readStringUntil('\n');
        receivedData.trim();

        Serial.print("STM32: ");
        Serial.println(receivedData);

        parseData();
    }
}

void handleData()
{
    String json = "{";

    json += "\"temp\":" + String(temp, 1) + ",";
    json += "\"hum\":" + String(hum, 1) + ",";
    json += "\"gas\":" + String(gasValue) + ",";
    json += "\"led\":\"" + ledStatus + "\"";

    json += "}";

    server.send(200, "application/json", json);
}

void handleLedOn()
{
    Serial2.println("LED_ON");
    ledStatus = "ON";

    server.send(200, "text/plain", "LED ON");
}

void handleLedOff()
{
    Serial2.println("LED_OFF");
    ledStatus = "OFF";

    server.send(200, "text/plain", "LED OFF");
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>BTL_VXL</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 20px;
            background-color: #f4f6f9;
            color: #333;
        }
        h1 { color: #2c3e50; margin-bottom: 20px; }
        .card {
            background: #ffffff;
            max-width: 420px;
            margin: 0 auto;
            padding: 25px 20px;
            border-radius: 15px;
            box-shadow: 0 4px 15px rgba(0,0,0,0.08);
        }
        h2 { font-size: 18px; color: #555; margin-bottom: 15px; }

        /* Khung công tắc gạt */
        .switch-container {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 15px;
            margin: 20px 0;
        }
        .switch {
            position: relative;
            display: inline-block;
            width: 64px;
            height: 36px;
        }
        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0; left: 0; right: 0; bottom: 0;
            background-color: #cbd5e1;
            transition: .3s;
            border-radius: 36px;
        }
        .slider:before {
            position: absolute;
            content: "";
            height: 28px;
            width: 28px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .3s;
            border-radius: 50%;
            box-shadow: 0 2px 4px rgba(0,0,0,0.2);
        }
        input:checked + .slider {
            background-color: #22c55e;
        }
        input:checked + .slider:before {
            transform: translateX(28px);
        }
        #ledStatus {
            font-weight: bold;
            font-size: 18px;
            color: #64748b;
            min-width: 50px;
            text-align: left;
        }

        hr {
            border: 0;
            border-top: 1px solid #e2e8f0;
            margin: 25px 0 20px;
        }

        .sensor-row {
            display: flex;
            justify-content: space-between;
            padding: 10px 15px;
            margin: 8px 0;
            background: #f8fafc;
            border-radius: 10px;
            font-size: 16px;
        }
        .sensor-val {
            font-weight: bold;
            color: #2563eb;
        }
    </style>
</head>

<body>
    <div class="card">
        <h1>🏠 SMART HOME ☀️</h1>

        <h2>Điều khiển LED</h2>
        <div class="switch-container">
            <label class="switch">
                <input type="checkbox" id="ledToggle" onchange="toggleLED(this)">
                <span class="slider"></span>
            </label>
            <span id="ledStatus">TẮT</span>
        </div>

        <hr>

        <h2>Thông số cảm biến</h2>
        <div class="sensor-row">
            <span>🌡️ Nhiệt độ:</span>
            <span><span id="temp" class="sensor-val">--</span> &deg;C</span>
        </div>
        <div class="sensor-row">
            <span>💧 Độ ẩm:</span>
            <span><span id="hum" class="sensor-val">--</span> %</span>
        </div>
        <div class="sensor-row">
            <span>⚠️ Gas:</span>
            <span><span id="gas" class="sensor-val">--</span></span>
        </div>
    </div>

    <script>
        async function updateData() {
            try {
                const response = await fetch('/data');
                const data = await response.json();

                document.getElementById('temp').textContent = data.temp;
                document.getElementById('hum').textContent = data.hum;
                document.getElementById('gas').textContent = data.gas;

                if (data.led) {
                    const isChecked = (data.led === 'ON');
                    document.getElementById('ledToggle').checked = isChecked;
                    const statusText = document.getElementById('ledStatus');
                    statusText.textContent = isChecked ? 'BẬT' : 'TẮT';
                    statusText.style.color = isChecked ? '#22c55e' : '#64748b';
                }
            } catch (err) {
                console.error("Lỗi cập nhật dữ liệu:", err);
            }
        }

        async function toggleLED(checkbox) {
            const state = checkbox.checked ? 'on' : 'off';
            const statusText = document.getElementById('ledStatus');
            statusText.textContent = checkbox.checked ? 'BẬT' : 'TẮT';
            statusText.style.color = checkbox.checked ? '#22c55e' : '#64748b';

            try {
                await fetch('/led/' + state);
            } catch (err) {
                console.error("Lỗi gửi lệnh LED:", err);
            }
        }

        updateData();
        setInterval(updateData, 1000);
    </script>
</body>
</html>
)rawliteral";

void handleRoot()
{
    server.send(200, "text/html; charset=utf-8", index_html);
}

void setup(){
    Serial.begin(115200);

    Serial2.begin(9600,SERIAL_8N1,STM_RX,STM_TX);

    WiFi.softAP(ssid,password);
    Serial.print("IP :");
    Serial.println(WiFi.softAPIP());

    server.on("/",handleRoot);
    server.on("/data",handleData);
    server.on("/led/on", handleLedOn);
    server.on("/led/off", handleLedOff);

    server.begin();
}

void loop(){
    readSTM();
    server.handleClient();
}
