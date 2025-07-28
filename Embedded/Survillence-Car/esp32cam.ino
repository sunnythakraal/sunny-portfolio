#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"

// WiFi credentials
const char* ssid = "ZTE_2.4G_jSHTMw";
const char* password = "6ifb3k6d";

// Web server
WebServer server(80);

// Camera pins
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

bool streaming = true;

void setup() {
  Serial.begin(115200);
  
  // Initialize camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 3;
  
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }
  
  // Fix orientation (adjust as needed)
  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1); // Flip vertically if upside-down
  s->set_hmirror(s, 1); // Mirror horizontally if reversed
  
  // Connect to WiFi
  IPAddress local_IP(192, 168, 1, 100);
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
  
  // Routes
  server.on("/stream", handleStream);
  server.begin();
}

void loop() {
  server.handleClient();
  
  // UART commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "STREAM_ON") streaming = true;
    else if (cmd == "STREAM_OFF") streaming = false;
  }
}

void handleStream() {
  if (!streaming) {
    server.send(503, "text/plain", "Streaming off");
    return;
  }
  
  WiFiClient client = server.client();
  server.sendContent("HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n");
  
  while (client.connected() && streaming) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) continue;
    
    String response = "--frame\r\nContent-Type: image/jpeg\r\n\r\n";
    client.print(response);
    client.write((char *)fb->buf, fb->len);
    client.print("\r\n");
    
    esp_camera_fb_return(fb);
    delay(50);
  }
}