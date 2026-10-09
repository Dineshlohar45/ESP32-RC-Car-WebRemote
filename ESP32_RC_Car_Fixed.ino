#include <WiFi.h>
#include <WebServer.h>

// =========================
// WiFi Connection Settings (Connect to YOUR WiFi/Hotspot)
// =========================
const char *ssid = "iPhone";          // Your iPhone hotspot name or WiFi SSID
const char *password = "DINU1979";    // Your iPhone hotspot password

// =========================
// Motor driver pins
// =========================
#define ENA 14   // Left motors PWM
#define IN1 27   // Left motor A
#define IN2 26   // Left motor B
#define ENB 32   // Right motors PWM
#define IN3 25   // Right motor A
#define IN4 33   // Right motor B

// =========================
// Motor speed
// =========================
const int MOTOR_SPEED = 200;  // 0 - 255

WebServer server(80);

// =========================
// Motor control functions
// =========================
void setLeftMotor(int speed, bool forward)
{
  analogWrite(ENA, abs(speed));
  digitalWrite(IN1, forward ? HIGH : LOW);
  digitalWrite(IN2, forward ? LOW : HIGH);
}

void setRightMotor(int speed, bool forward)
{
  analogWrite(ENB, abs(speed));
  digitalWrite(IN3, forward ? HIGH : LOW);
  digitalWrite(IN4, forward ? LOW : HIGH);
}

void moveForward()
{
  setLeftMotor(MOTOR_SPEED, true);
  setRightMotor(MOTOR_SPEED, true);
}

void moveBackward()
{
  setLeftMotor(MOTOR_SPEED, false);
  setRightMotor(MOTOR_SPEED, false);
}

void turnLeft()
{
  // Left side reverse, right side forward
  setLeftMotor(MOTOR_SPEED, false);
  setRightMotor(MOTOR_SPEED, true);
}

void turnRight()
{
  // Left side forward, right side reverse
  setLeftMotor(MOTOR_SPEED, true);
  setRightMotor(MOTOR_SPEED, false);
}

void stopCar()
{
  setLeftMotor(0, true);
  setRightMotor(0, true);
}

// =========================
// HTML page for remote control
// =========================
String getRemotePage()
{
  return R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>ESP32 RC Car</title>
  <style>
    :root {
      --bg1: #0f172a;
      --bg2: #111827;
      --panel: rgba(17, 24, 39, 0.8);
      --primary: #38bdf8;
      --secondary: #8b5cf6;
      --accent: #22c55e;
      --warn: #f59e0b;
      --danger: #ef4444;
      --text: #e5eefb;
      --muted: #9fb3c8;
      --shadow: rgba(0, 0, 0, 0.35);
    }

    * {
      box-sizing: border-box;
      -webkit-tap-highlight-color: transparent;
      user-select: none;
    }

    html, body {
      margin: 0;
      height: 100%;
      font-family: Arial, Helvetica, sans-serif;
      background: radial-gradient(circle at top, #1d4ed8 0%, #0f172a 35%, #020817 100%);
      color: var(--text);
      display: flex;
      justify-content: center;
      align-items: center;
    }

    body {
      padding: 16px;
    }

    .container {
      width: min(420px, 100%);
      background: rgba(15, 23, 42, 0.82);
      border: 1px solid rgba(148, 163, 184, 0.2);
      border-radius: 24px;
      box-shadow: 0 30px 80px rgba(0, 0, 0, 0.45);
      padding: 22px 18px 18px;
      backdrop-filter: blur(12px);
    }

    .title {
      text-align: center;
      font-size: clamp(1.5rem, 5vw, 2.2rem);
      font-weight: 800;
      letter-spacing: 1px;
      margin-bottom: 18px;
      background: linear-gradient(90deg, var(--primary), #7dd3fc, var(--secondary));
      -webkit-background-clip: text;
      background-clip: text;
      color: transparent;
      text-shadow: 0 0 18px rgba(56, 189, 248, 0.25);
    }

    .status {
      text-align: center;
      color: var(--muted);
      font-size: 0.9rem;
      margin-bottom: 20px;
      letter-spacing: 0.08em;
      text-transform: uppercase;
    }

    .pad {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 14px;
      max-width: 340px;
      margin: 0 auto;
    }

    .btn {
      border: none;
      border-radius: 18px;
      min-height: 74px;
      color: white;
      font-size: 1.05rem;
      font-weight: 700;
      letter-spacing: 0.03em;
      cursor: pointer;
      box-shadow: 0 10px 22px rgba(0,0,0,0.25);
      transition: transform 0.08s ease, box-shadow 0.2s ease, filter 0.2s ease;
      touch-action: manipulation;
    }

    .btn:active, .btn.active {
      transform: scale(0.97);
      box-shadow: inset 0 4px 12px rgba(255,255,255,0.12), 0 6px 16px rgba(0,0,0,0.25);
      filter: brightness(1.08);
    }

    .forward {
      background: linear-gradient(135deg, var(--primary), #0ea5e9);
      grid-column: 2;
      grid-row: 1;
    }

    .left {
      background: linear-gradient(135deg, var(--secondary), #7c3aed);
      grid-column: 1;
      grid-row: 2;
    }

    .stop {
      background: linear-gradient(135deg, var(--danger), #dc2626);
      grid-column: 2;
      grid-row: 2;
    }

    .right {
      background: linear-gradient(135deg, var(--warn), #f97316);
      grid-column: 3;
      grid-row: 2;
    }

    .backward {
      background: linear-gradient(135deg, var(--accent), #16a34a);
      grid-column: 2;
      grid-row: 3;
    }

    .info {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-top: 18px;
      padding: 10px 12px;
      border-radius: 14px;
      font-size: 0.82rem;
      background: rgba(15, 118, 110, 0.16);
      border: 1px solid rgba(45, 212, 191, 0.2);
      color: var(--muted);
    }

    .dot {
      width: 12px;
      height: 12px;
      border-radius: 50%;
      background: #22c55e;
      box-shadow: 0 0 12px rgba(34, 197, 94, 0.8);
      display: inline-block;
      margin-right: 8px;
      vertical-align: middle;
    }

    .ip-display {
      text-align: center;
      margin-top: 12px;
      padding: 8px;
      background: rgba(56, 189, 248, 0.1);
      border-radius: 8px;
      font-size: 0.9rem;
      color: var(--primary);
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="title">ESP32 RC CAR</div>
    <div class="status">Wi‑Fi Remote Controller</div>

    <div class="pad">
      <button class="btn forward" data-cmd="forward">▲ FORWARD</button>
      <button class="btn left" data-cmd="left">◀ LEFT</button>
      <button class="btn stop" data-cmd="stop">■ STOP</button>
      <button class="btn right" data-cmd="right">RIGHT ▶</button>
      <button class="btn backward" data-cmd="backward">▼ BACK</button>
    </div>

    <div class="info">
      <span><span class="dot"></span>Connected</span>
      <span id="status-text">Loading...</span>
    </div>

    <div class="ip-display" id="ip-info">
      Getting IP Address...
    </div>
  </div>

  <script>
    const buttons = document.querySelectorAll('.btn');

    function sendCommand(cmd) {
      fetch('/move?cmd=' + cmd, { method: 'GET' })
        .then(res => res.text())
        .catch(err => console.log('Command error:', err));
    }

    function activateButton(btn) {
      buttons.forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
    }

    function releaseAll() {
      buttons.forEach(b => b.classList.remove('active'));
      sendCommand('stop');
    }

    buttons.forEach((btn) => {
      const cmd = btn.dataset.cmd;

      const start = (e) => {
        e.preventDefault();
        activateButton(btn);
        sendCommand(cmd);
      };

      const end = (e) => {
        e.preventDefault();
        releaseAll();
      };

      btn.addEventListener('pointerdown', start);
      btn.addEventListener('pointerup', end);
      btn.addEventListener('pointerleave', end);
      btn.addEventListener('pointercancel', end);

      btn.addEventListener('touchstart', start, { passive: false });
      btn.addEventListener('touchend', end, { passive: false });
      btn.addEventListener('contextmenu', (e) => e.preventDefault());
    });

    document.addEventListener('keydown', (event) => {
      const key = event.key.toLowerCase();
      const map = {
        'w': 'forward',
        's': 'backward',
        'a': 'left',
        'd': 'right',
        ' ': 'stop'
      };

      if (map[key]) {
        event.preventDefault();
        const current = document.querySelector('[data-cmd="' + map[key] + '"]');
        if (current) {
          activateButton(current);
          sendCommand(map[key]);
        }
      }
    });

    document.addEventListener('keyup', (event) => {
      const key = event.key.toLowerCase();
      if (key === 'w' || key === 's' || key === 'a' || key === 'd' || key === ' ') {
        releaseAll();
      }
    });

    // Get ESP32 local IP
    fetch('/getip')
      .then(res => res.text())
      .then(ip => {
        document.getElementById('ip-info').innerHTML = 'IP: ' + ip;
        document.getElementById('status-text').innerHTML = ip.split(':')[0];
      })
      .catch(() => {
        document.getElementById('ip-info').innerHTML = 'Connected to WiFi';
      });
  </script>
</body>
</html>
)HTML";
}

// =========================
// Web server handlers
// =========================
void handleRoot()
{
  server.send(200, "text/html", getRemotePage());
}

void handleMove()
{
  String cmd = server.arg("cmd");

  if (cmd == "forward") {
    moveForward();
  }
  else if (cmd == "backward") {
    moveBackward();
  }
  else if (cmd == "left") {
    turnLeft();
  }
  else if (cmd == "right") {
    turnRight();
  }
  else if (cmd == "stop") {
    stopCar();
  }

  server.send(200, "text/plain", "OK");
}

void handleGetIP()
{
  String ip = WiFi.localIP().toString();
  server.send(200, "text/plain", ip);
}

void handleNotFound()
{
  server.send(404, "text/plain", "Not found");
}

// =========================
// Setup
// =========================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopCar();

  // Connect to WiFi in STATION mode (connect to your WiFi/Hotspot)
  WiFi.mode(WIFI_STA);
  
  Serial.println();
  Serial.println("========================================");
  Serial.println("ESP32 RC Car - Station Mode");
  Serial.println("========================================");
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✓ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("✗ Failed to connect to WiFi");
    Serial.println("Falling back to Access Point mode...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_RC_Car", "12345678");
    Serial.println("Access Point created: ESP32_RC_Car");
    Serial.println("Password: 12345678");
  }

  Serial.println("========================================");
  Serial.print("Remote URL: http://");
  Serial.println(WiFi.localIP());
  Serial.println("========================================");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/move", HTTP_GET, handleMove);
  server.on("/getip", HTTP_GET, handleGetIP);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

// =========================
// Loop
// =========================
void loop()
{
  server.handleClient();
}