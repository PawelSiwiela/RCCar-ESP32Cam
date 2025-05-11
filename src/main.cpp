#include <WiFi.h>
#include <WebServer.h>
#include <esp_camera.h>

const char* ssid = "Pae";
const char* password = "20022005ab";

WebServer server(80);

// Konfiguracja pinów ESP32-CAM
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

void initCamera() {
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
  
  // Zoptymalizowane parametry kamery
  config.xclk_freq_hz = 20000000;  // Zwiększona częstotliwość zegara
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;  // Zmniejszona rozdzielczość dla lepszej wydajności
  config.jpeg_quality = 10;           // Lekko obniżona jakość dla szybszego przesyłu
  config.fb_count = 2;               // Dwa bufory dla lepszej płynności
  config.grab_mode = CAMERA_GRAB_LATEST; // Zawsze bierzemy najnowszą klatkę

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Błąd inicjalizacji kamery (0x%x). Resetowanie...\n", err);
    delay(1000);
    ESP.restart();
  }
  
  // Zoptymalizowane parametry sensora
  sensor_t * s = esp_camera_sensor_get();
  if (s) {
    s->set_brightness(s, 1);     // Lekko zwiększona jasność
    s->set_contrast(s, 1);       // Lekko zwiększony kontrast
    s->set_saturation(s, 1);     // Lekko zwiększona saturacja
    s->set_special_effect(s, 0); // Bez efektów
    s->set_whitebal(s, 1);       // Auto white balance
    s->set_awb_gain(s, 1);       // Auto white balance gain
    s->set_exposure_ctrl(s, 1);  // Auto exposure
    s->set_gainceiling(s, GAINCEILING_2X); // Zwiększony gain dla lepszej jakości w słabym świetle
    s->set_vflip(s, 0);         // Wyłącz odbicie pionowe jeśli potrzebne
    s->set_hmirror(s, 0);       // Wyłącz odbicie poziome jeśli potrzebne
  }
}

void handleRoot() {
  String html = "<html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body{margin:0;background:#000;display:flex;flex-direction:column;align-items:center;}";
  html += "img{max-width:100%;height:auto;object-fit:contain;}";
  html += "</style></head><body>";
  html += "<img id='stream' src='/stream'>";
  html += "<script>";
  html += "const img=document.getElementById('stream');";
  html += "let loading=false;";
  html += "function updateImage(){";
  html += "  if(loading)return;";
  html += "  loading=true;";
  html += "  img.src='/stream?'+Date.now();";
  html += "  img.onload=()=>loading=false;";
  html += "  img.onerror=()=>loading=false;";
  html += "}";
  html += "setInterval(updateImage,100);"; // Częstsze odświeżanie - co 100ms
  html += "</script></body></html>";
  server.send(200, "text/html", html);
}

void handleStream() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Błąd pobrania obrazu");
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Disposition", "inline");
  server.sendHeader("Content-Length", String(fb->len));
  
  server.send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);
  
  // Inicjalizacja kamery
  initCamera();
  
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // Wyłącz tryb oszczędzania energii WiFi
  WiFi.begin(ssid, password);
  
  Serial.print("Łączenie z Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Połączono z Wi-Fi");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/stream", handleStream);
  server.begin();
  Serial.println("Serwer HTTP uruchomiony");
}

void loop() {
  server.handleClient();
  delay(1); // Minimalne opóźnienie dla stabilności
}
