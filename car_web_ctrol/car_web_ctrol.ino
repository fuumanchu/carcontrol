

#include <WiFi.h>
#include <WebServer.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#define TYPE_CAR_CONTROL  1


typedef union {
    struct {
        int speed; 
        char direction[4]; 
        int lights; 
        int horn;
    } data;
    uint8_t buffer[sizeof(int) + 4 + sizeof(int) * 2]; 
} CarControl;


typedef struct {
    uint8_t type; 
    CarControl control;
} CarMessage;

uint8_t txBuffer[sizeof(CarMessage)];

void sendCarData(uint8_t type, int speed, const char *direction, int lights, int horn) {
    CarMessage carMsg;
    carMsg.type = type; 
    carMsg.control.data.speed = speed;
    strcpy(carMsg.control.data.direction, direction);
    carMsg.control.data.lights = lights;
    carMsg.control.data.horn = horn;
    memcpy(txBuffer, &carMsg, sizeof(CarMessage));
    Serial.write(txBuffer, sizeof(txBuffer));
}

// Thông tin WiFi - thay đổi thông tin của bạn vào đây
const char* ssid = "ACTVN-TA1";
const char* password = "12345679";

// Biến toàn cục lưu trạng thái

int speed = 50;            // Tốc độ mặc định (0-1023)
char direc[4]; 
int lightState = 0;    // Trạng thái đèn
int hornState = 0;     // Trạng thái còi

// Khởi tạo server tại cổng 80
WebServer server(80);

void handleForward(); 
void handleBackward(); 
void handleLeft(); 
void handleRight(); 
void handleStop(); 
void handleSpeedUp(); 
void handleSpeedDown(); 
void handleLight(); 
void handleHorn(); 
void handleStatus(); 

void setup() {
  // Khởi tạo Serial để debug
  Serial.begin(115200);
  Serial.println();
  // Kết nối WiFi
  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối đến ");
  Serial.print(ssid);
  
  // Chờ kết nối WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Đã kết nối, địa chỉ IP: ");
  Serial.println(WiFi.localIP());
  
  // Định nghĩa các endpoint API
  server.on("/", handleRoot);                  // Trang chủ
  server.on("/forward", handleForward);        // Đi tiến
  server.on("/backward", handleBackward);      // Đi lùi
  server.on("/left", handleLeft);              // Rẽ trái
  server.on("/right", handleRight);            // Rẽ phải
  server.on("/stop", handleStop);              // Dừng lại
  server.on("/speed/up", handleSpeedUp);       // Tăng tốc độ
  server.on("/speed/down", handleSpeedDown);   // Giảm tốc độ
  server.on("/light/toggle", handleLight);     // Bật/tắt đèn
  server.on("/horn/toggle", handleHorn);       // Bật/tắt còi
  server.on("/status", handleStatus);          // Trả về trạng thái hiện tại
  
  // Bắt đầu web server
  server.begin();
}

void loop() {
  server.handleClient();  // Xử lý các request từ client
}

// Xử lý trang chủ
void handleRoot() {
  String html = R"html(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Điều Khiển Xe</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 20px;
            background-color: #f0f0f0;
        }
        
        .container {
            max-width: 600px;
            margin: 0 auto;
            background-color: white;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }
        
        h1 {
            color: #333;
        }
        
        .status {
            margin: 20px 0;
            padding: 10px;
            background-color: #e9e9e9;
            border-radius: 5px;
        }
        
        .control-group {
            margin: 15px 0;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        
        .direction-pad {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            grid-gap: 10px;
            margin-bottom: 20px;
        }
        
        .options {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            grid-gap: 10px;
            margin-bottom: 20px;
        }
        
        button {
            padding: 15px;
            font-size: 16px;
            border: none;
            border-radius: 8px;
            margin: 5px;
            cursor: pointer;
            background-color: #4CAF50;
            color: white;
            min-width: 80px;
            transition: background-color 0.3s;
        }
        
        button:hover {
            background-color: #45a049;
        }
        
        button:active {
            background-color: #3e8e41;
        }
        
        .empty {
            visibility: hidden;
        }
        
        .forward {
            grid-column: 2;
            grid-row: 1;
        }
        
        .backward {
            grid-column: 2;
            grid-row: 3;
        }
        
        .left {
            grid-column: 1;
            grid-row: 2;
        }
        
        .right {
            grid-column: 3;
            grid-row: 2;
        }
        
        .stop {
            grid-column: 2;
            grid-row: 2;
            background-color: #f44336;
        }
        
        .stop:hover {
            background-color: #d32f2f;
        }
        
        .speed-up {
            background-color: #2196F3;
        }
        
        .speed-up:hover {
            background-color: #0b7dda;
        }
        
        .speed-down {
            background-color: #2196F3;
        }
        
        .speed-down:hover {
            background-color: #0b7dda;
        }
        
        .light {
            background-color: #FF9800;
        }
        
        .light:hover {
            background-color: #e68a00;
        }
        
        .horn {
            background-color: #9C27B0;
        }
        
        .horn:hover {
            background-color: #7B1FA2;
        }
        
        .active {
            box-shadow: 0 0 10px 3px rgba(0, 123, 255, 0.6);
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Điều Khiển Xe ESP8266</h1>
        
        <div class="status">
            <p>Tốc độ: <span id="speed-value">50</span>%</p>
            <p>Đèn: <span id="light-status">Tắt</span></p>
            <p>Còi: <span id="horn-status">Tắt</span></p>
        </div>
        
        <div class="control-group">
            <h2>Điều Hướng</h2>
            <div class="direction-pad">
                <div class="empty"></div>
                <button class="forward" id="forward" ontouchstart="sendCommand('forward')" ontouchend="sendCommand('stop')" onmousedown="sendCommand('forward')" onmouseup="sendCommand('stop')">Tiến</button>
                <div class="empty"></div>
                <button class="left" id="left" ontouchstart="sendCommand('left')" ontouchend="sendCommand('stop')" onmousedown="sendCommand('left')" onmouseup="sendCommand('stop')">Trái</button>
                <button class="stop" id="stop" onclick="sendCommand('stop')">Dừng</button>
                <button class="right" id="right" ontouchstart="sendCommand('right')" ontouchend="sendCommand('stop')" onmousedown="sendCommand('right')" onmouseup="sendCommand('stop')">Phải</button>
                <div class="empty"></div>
                <button class="backward" id="backward" ontouchstart="sendCommand('backward')" ontouchend="sendCommand('stop')" onmousedown="sendCommand('backward')" onmouseup="sendCommand('stop')">Lùi</button>
                <div class="empty"></div>
            </div>
        </div>
        
        <div class="control-group">
            <h2>Tùy Chọn</h2>
            <div class="options">
                <button class="speed-up" id="speed-up" onclick="sendCommand('speed/up')">Tăng Tốc</button>
                <button class="speed-down" id="speed-down" onclick="sendCommand('speed/down')">Giảm Tốc</button>
                <button class="light" id="light" onclick="sendCommand('light/toggle')">Đèn</button>
                <button class="horn" id="horn" onclick="sendCommand('horn/toggle')">Còi</button>
            </div>
        </div>
    </div>
    
    <script>
        // Khởi tạo trạng thái
        let state = {
            speed: 50,
            light: false,
            horn: false
        };
        
        // Cập nhật trạng thái từ server khi tải trang
        window.onload = function() {
            updateStatus();
            
            // Cập nhật trạng thái mỗi 5 giây
            setInterval(updateStatus, 5000);
        };
        
        // Hàm cập nhật trạng thái
        function updateStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    state = data;
                    document.getElementById('speed-value').textContent = state.speed;
                    document.getElementById('light-status').textContent = state.light ? 'Bật' : 'Tắt';
                    document.getElementById('horn-status').textContent = state.horn ? 'Bật' : 'Tắt';
                    
                    // Cập nhật trạng thái visual
                    document.getElementById('light').classList.toggle('active', state.light);
                    document.getElementById('horn').classList.toggle('active', state.horn);
                })
                .catch(error => console.error('Lỗi:', error));
        }
        
        // Hàm gửi lệnh đến server
        function sendCommand(command) {
            fetch('/' + command)
                .then(response => response.text())
                .then(data => {
                    console.log('Phản hồi:', data);
                    updateStatus();
                })
                .catch(error => console.error('Lỗi:', error));
        }
    </script>
</body>
</html>
  )html";
  
  server.send(200, "text/html", html);
}

// Xử lý API trả về trạng thái hiện tại
void handleStatus() {
  String json = "{";
  json += "\"speed\":" + String(speed);
  json += ", \"light\":" + String(lightState ? "true" : "false");
  json += ", \"horn\":" + String(hornState ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

// Các hàm xử lý điều khiển xe

void handleForward() {
  strcpy(direc, "up");
  sendCarData(1,speed,direc,lightState,hornState); 
  server.send(200, "text/plain", "OK");
}

void handleBackward() {
  strcpy(direc, "down");
  sendCarData(1,speed,direc,lightState,hornState);
  server.send(200, "text/plain", "OK");
}

void handleLeft() {
  strcpy(direc, "left");

  sendCarData(1,speed,direc,lightState,hornState);
  
  server.send(200, "text/plain", "OK");
}

void handleRight() {
  
  // Động cơ A: Tiến
  strcpy(direc, "righ");
 
  sendCarData(1,speed,direc,lightState,hornState);
  server.send(200, "text/plain", "OK");
}

void handleStop() {
  strcpy(direc, "stop");

  sendCarData(1,speed,direc,lightState,hornState);
  
  server.send(200, "text/plain", "OK");
}

void handleSpeedUp() {
  // Tăng tốc độ lên 10%
  if(speed != 100){
    speed += 10; 
    sendCarData(1,speed,"a",lightState,hornState);
  }
  server.send(200, "text/plain", "OK");
  
}

void handleSpeedDown() {
  // Giảm tốc độ xuống 10%
  if(speed != 0){
    speed -= 10; 
    sendCarData(1,speed,"a",lightState,hornState);
  }
  
  server.send(200, "text/plain", "OK");
}

void handleLight() {
  // Đảo trạng thái đèn
  lightState = !lightState;
  sendCarData(1,speed,"none",lightState,hornState);
  if(lightState == 1) {
    Serial.println("den on"); 
  }
  else if(lightState == 0){
    Serial.println("den off");
  }
  
  server.send(200, "text/plain", "OK");
}

void handleHorn() {
  // Đảo trạng thái còi
  hornState = !hornState;
  sendCarData(1,speed,"",lightState,hornState);
  server.send(200, "text/plain", "OK");
}


