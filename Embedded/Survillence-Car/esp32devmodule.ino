#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "ZTE_2.4G_jSHTMw";
const char* password = "6ifb3k6d";

// Pins
#define DHTPIN 4
#define MQ2PIN 34
#define IN1 27
#define IN2 26
#define IN3 25
#define IN4 33
#define UART_TX 17
#define UART_RX 16

// DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Web server
WebServer server(80);
const char* camStreamUrl = "http://192.168.1.100/stream";

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, UART_RX, UART_TX);
  
  // Initialize pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(MQ2PIN, INPUT);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  
  // Initialize DHT
  dht.begin();
  
  // Connect to WiFi
  IPAddress local_IP(192, 168, 1, 101);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());
  
  // Start CAM stream
  Serial2.println("STREAM_ON");
  
  // Routes
  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.begin();
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>RC Car</title>";
  html += "<style>body{font-family:Arial;text-align:center}";
  html += ".c{padding:20px}img{max-width:640px}";
  html += ".b{display:inline-block;padding:15px 30px;margin:10px;font-size:18px;";
  html += "cursor:pointer;background:#4CAF50;color:#fff;border:0;border-radius:5px}";
  html += ".s{margin:20px;font-size:18px}</style></head>";
  html += "<body><div class=c><h1>RC Car</h1>";
  html += "<img src=\"" + String(camStreamUrl) + "\" id=cam>";
  html += "<div class=s id=sensors>Loading...</div>";
  html += "<div>";
  html += "<div class=b onmousedown=\"sendControl('forward')\" onmouseup=\"sendControl('stop')\">Fwd</div>";
  html += "<div class=b onmousedown=\"sendControl('backward')\" onmouseup=\"sendControl('stop')\">Back</div>";
  html += "<div class=b onmousedown=\"sendControl('right')\" onmouseup=\"sendControl('stop')\">Left</div>";
  html += "<div class=b onmousedown=\"sendControl('left')\" onmouseup=\"sendControl('stop')\">Right</div>";
  html += "</div></div><script>";
  html += "async function sendControl(d){await fetch('/control?dir='+d);}";
  html += "async function updateSensors(){";
  html += "let r=await fetch('/control?dir=sensor');";
  html += "let d=await r.json();";
  html += "document.getElementById('sensors').innerHTML=";
  html += "T: ${d.temp}°C<br>H: ${d.hum}%<br>G: ${d.gas};";
  html += "}";
  html += "setInterval(updateSensors,15000);";
  html += "updateSensors();";
  html += "</script></body></html>";
  
  server.send(200, "text/html", html);
}

void handleControl() {
  String dir = server.arg("dir");
  
  if (dir == "forward") {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else if (dir == "backward") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else if (dir == "left") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else if (dir == "right") {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else if (dir == "stop") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  } else if (dir == "sensor") {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int gas = analogRead(MQ2PIN);
    
    String json = "{\"temp\":" + String(temp) + ",\"hum\":" + String(hum) + ",\"gas\":" + String(gas) + "}";
    server.send(200, "application/json", json);
    return;
  }
  
  server.send(200, "text/plain", "OK");
}