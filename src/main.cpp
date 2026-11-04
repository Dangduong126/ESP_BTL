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


//Ham bat led
void handleLedOn()
{
    Serial2.println("LED_ON");
    Serial.println("Nhan lenh BAT LED");
    ledStatus = "ON";

    server.send(200, "text/plain", "LED ON");
}


//Ham tat led
void handleLedOff()
{
    Serial2.println("LED_OFF");
    Serial.println("Nhan lenh TAT LED");
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
        * { box-sizing: border-box; }
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 20px;
            background-color: #f4f6f9;
            color: #333;
        }
        h1 { color: #2c3e50; margin-bottom: 25px; font-size: 26px; }
        .container {
            display: flex;
            justify-content: center;
            align-items: stretch;
            gap: 20px;
            max-width: 780px;
            margin: 0 auto;
            flex-wrap: wrap;
        }
        .card {
            background: #ffffff;
            flex: 1 1 320px;
            max-width: 380px;
            padding: 25px 20px;
            border-radius: 16px;
            box-shadow: 0 4px 15px rgba(0,0,0,0.07);
            display: flex;
            flex-direction: column;
            justify-content: space-between;
        }
        h2 {
            font-size: 18px;
            color: #334155;
            margin-top: 0;
            margin-bottom: 18px;
            padding-bottom: 10px;
            border-bottom: 2px solid #f1f5f9;
        }
        .switch-box {
            flex: 1;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }
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
            width: 66px;
            height: 36px;
        }
        .switch input { opacity: 0; width: 0; height: 0; }
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
        input:checked + .slider { background-color: #22c55e; }
        input:checked + .slider:before { transform: translateX(30px); }
        #ledStatus {
            font-weight: bold;
            font-size: 19px;
            color: #64748b;
            min-width: 55px;
            text-align: left;
        }
        .sensor-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 12px 16px;
            margin: 10px 0;
            background: #f8fafc;
            border-radius: 12px;
            font-size: 16px;
            border: 1px solid #f1f5f9;
            transition: background 0.3s, border-color 0.3s;
        }
        .sensor-row.warn-yellow { background: #fefce8; border-color: #fde047; }
        .sensor-row.warn-red   { background: #fef2f2; border-color: #fca5a5; animation: blink-border 1s infinite; }
        @keyframes blink-border {
            0%, 100% { border-color: #fca5a5; }
            50%       { border-color: #ef4444; }
        }
        .sensor-val { font-weight: bold; color: #2563eb; font-size: 17px; }
        .sensor-val.warn-yellow { color: #ca8a04; }
        .sensor-val.warn-red    { color: #dc2626; }

        /* ===== TOAST POPUP ===== */
        #toast-container {
            position: fixed;
            top: 20px;
            right: 20px;
            display: flex;
            flex-direction: column;
            gap: 10px;
            z-index: 9999;
        }
        .toast {
            display: flex;
            align-items: flex-start;
            gap: 12px;
            min-width: 280px;
            max-width: 360px;
            padding: 14px 16px;
            border-radius: 14px;
            box-shadow: 0 8px 24px rgba(0,0,0,0.15);
            font-size: 14px;
            line-height: 1.4;
            animation: slide-in 0.35s ease;
            cursor: pointer;
        }
        .toast.warning {
            background: #fffbeb;
            border-left: 5px solid #f59e0b;
            color: #78350f;
        }
        .toast.danger {
            background: #fff1f2;
            border-left: 5px solid #ef4444;
            color: #7f1d1d;
        }
        .toast-icon { font-size: 22px; flex-shrink: 0; margin-top: 1px; }
        .toast-body { flex: 1; }
        .toast-title { font-weight: bold; font-size: 15px; margin-bottom: 2px; }
        .toast-close {
            background: none;
            border: none;
            font-size: 18px;
            cursor: pointer;
            color: inherit;
            opacity: 0.5;
            line-height: 1;
            padding: 0;
        }
        .toast-close:hover { opacity: 1; }
        @keyframes slide-in {
            from { opacity: 0; transform: translateX(60px); }
            to   { opacity: 1; transform: translateX(0);    }
        }
    </style>
</head>

<body>
    <h1>HOME: ROOM 1</h1>

    <!-- Khu vực hiển thị popup toast -->
    <div id="toast-container"></div>

    <div class="container">
        <!-- Khung bên trái: Thông số cảm biến -->
        <div class="card">
            <h2>Dữ Liệu Cảm Biến</h2>
            <div>
                <div class="sensor-row" id="row-temp">
                    <span>🌡️ Nhiệt độ:</span>
                    <span><span id="temp" class="sensor-val">--</span> &deg;C</span>
                </div>
                <div class="sensor-row" id="row-hum">
                    <span>💧 Độ ẩm:</span>
                    <span><span id="hum" class="sensor-val">--</span> %</span>
                </div>
                <div class="sensor-row" id="row-gas">
                    <span>⚠️ GAS:</span>
                    <span><span id="gas" class="sensor-val">--</span> ppm</span>
                </div>
            </div>
        </div>

        <!-- Khung bên phải: Điều khiển LED -->
        <div class="card">
            <h2>Điều khiển LED</h2>
            <div class="switch-box">
                <div class="switch-container">
                    <label class="switch">
                        <input type="checkbox" id="ledToggle" onchange="toggleLED(this)">
                        <span class="slider"></span>
                    </label>
                    <span id="ledStatus">TẮT</span>
                </div>
            </div>
        </div>
    </div>

    <script>
        // ===== Ngưỡng cảnh báo =====
        const TEMP_WARN  = 40;    // °C - nhiệt độ cao
        const GAS_WARN   = 300;   // ppm - cảnh báo
        const GAS_DANGER = 1000;  // ppm - nguy hiểm

        // ===== Quản lý toast popup =====
        // userDismissed: ghi nhớ người dùng đã tắt toast nào (theo id toast)
        // Khi trạng thái cảnh báo đổi mức (vd: gasWarn -> gasDanger), id thay đổi
        // nên popup mới vẫn hiện dù người dùng đã tắt mức cũ
        const userDismissed = {};

        // showOrUpdateToast: tạo mới hoặc cập nhật nội dung toast theo group (temp / gas)
        // group: nhóm logic ('temp', 'gas') — mỗi nhóm chỉ có 1 toast tại 1 thời điểm
        // id: định danh mức cụ thể ('temp', 'gasWarn', 'gasDanger')
        function showOrUpdateToast(group, id, type, icon, title, message) {
            const container = document.getElementById('toast-container');
            const domId     = 'toast-' + group;

            // Nếu người dùng đã tắt đúng mức này, không hiện lại
            if (userDismissed[id]) return;

            let toast = document.getElementById(domId);

            if (!toast) {
                // Tạo toast mới
                toast = document.createElement('div');
                toast.id = domId;
                container.appendChild(toast);
            }

            // Cập nhật nội dung & kiểu (kể cả khi đã tồn tại — để đổi mức warn <-> danger)
            toast.className = 'toast ' + type;
            toast.dataset.alertId = id;   // lưu mức hiện tại để biết người dùng tắt mức nào
            toast.innerHTML =
                '<span class="toast-icon">' + icon + '</span>' +
                '<div class="toast-body">' +
                    '<div class="toast-title">' + title + '</div>' +
                    '<div>' + message + '</div>' +
                '</div>' +
                '<button class="toast-close" onclick="dismissToast(\'' + group + '\')">✕</button>';
        }

        // dismissToast: người dùng chủ động tắt — ghi nhớ mức đang hiện để không spam lại
        function dismissToast(group) {
            const toast = document.getElementById('toast-' + group);
            if (!toast) return;
            const currentId = toast.dataset.alertId;
            if (currentId) userDismissed[currentId] = true;
            toast.remove();
        }

        // removeToast: hệ thống tự xóa (khi điều kiện hết) — KHÔNG ghi nhớ userDismissed
        function removeToast(group) {
            const toast = document.getElementById('toast-' + group);
            if (toast) toast.remove();
        }

        // ===== Cập nhật màu dòng cảm biến =====
        function setRowLevel(rowId, valId, level) {
            const row = document.getElementById(rowId);
            const val = document.getElementById(valId);
            row.className = 'sensor-row' + (level ? ' warn-' + level : '');
            val.className = 'sensor-val' + (level ? ' warn-' + level : '');
        }

        // ===== Kiểm tra ngưỡng & hiện toast =====
        function checkAlerts(temp, gas) {
            // --- Nhiệt độ ---
            if (temp > TEMP_WARN) {
                setRowLevel('row-temp', 'temp', 'red');
                showOrUpdateToast('temp', 'temp', 'danger', '🌡️',
                    'Nhiệt độ quá cao!',
                    'Nhiệt độ hiện tại: ' + temp + ' °C (ngưỡng > ' + TEMP_WARN + ' °C)');
            } else {
                setRowLevel('row-temp', 'temp', '');
                // Hết cảnh báo: reset dismissed để lần sau vượt ngưỡng sẽ báo lại
                delete userDismissed['temp'];
                // KHÔNG tự đóng toast — để người dùng tự tắt
            }

            // --- Khí gas ---
            if (gas >= GAS_DANGER) {
                setRowLevel('row-gas', 'gas', 'red');
                // Xóa cờ mức thấp hơn để nếu về warn sau sẽ báo lại
                delete userDismissed['gasWarn'];
                showOrUpdateToast('gas', 'gasDanger', 'danger', '💨',
                    'Nguy hiểm! Nồng độ khí gas cao!',
                    'Gas: ' + gas + ' ppm — Mở cửa sổ, tắt lửa, rời khỏi khu vực ngay!');
            } else if (gas >= GAS_WARN) {
                setRowLevel('row-gas', 'gas', 'yellow');
                // Xóa cờ mức cao hơn để nếu leo lại sẽ báo lại
                delete userDismissed['gasDanger'];
                showOrUpdateToast('gas', 'gasWarn', 'warning', '⚠️',
                    'Phát hiện khí gas bất thường',
                    'Gas: ' + gas + ' ppm — Hãy kiểm tra khu vực xung quanh.');
            } else {
                setRowLevel('row-gas', 'gas', '');
                // Hết cảnh báo: reset dismissed để lần sau có thể báo lại
                delete userDismissed['gasWarn'];
                delete userDismissed['gasDanger'];
                // KHÔNG tự đóng toast — để người dùng tự tắt
            }
        }

        // ===== Cập nhật dữ liệu từ ESP32 =====
        async function updateData() {
            try {
                const response = await fetch('/data');
                const data = await response.json();

                document.getElementById('temp').textContent = data.temp;
                document.getElementById('hum').textContent = data.hum;
                document.getElementById('gas').textContent = data.gas;

                checkAlerts(parseFloat(data.temp), parseInt(data.gas));

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
