#include <WiFi.h>
#include <WebServer.h>
#include <esp_camera.h>

const char* ssid = "S23";
const char* password = "1234567890";

WebServer server(80);

// Globalne zmienne do przechowywania prędkości i dystansu
float carSpeed = 0.0;
float carDistance = 0.0;
unsigned long lastSpeedDataTime = 0;

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
  html += ".telemetry{background-color:#333;color:white;padding:10px;margin-top:10px;border-radius:5px;width:80%;max-width:600px;font-family:Arial,sans-serif;}";
  html += ".telemetry h3{margin:0 0 10px 0;text-align:center;}";
  html += ".value{font-size:18px;margin:5px 0;}";
  html += ".stale{color:#ff5555;}";
  html += "</style></head><body>";
  html += "<img id='stream' src='/stream'>";
  
  // Dodaj sekcję z prędkością i dystansem
  html += "<div class='telemetry'>";
  html += "<h3>Car Telemetry</h3>";
  html += "<p class='value'>Speed: <span id='speed'>" + String(carSpeed, 1) + "</span> km/h</p>";
  html += "<p class='value'>Distance: <span id='distance'>" + String(carDistance, 1) + "</span> m</p>";
  
  // Sprawdź czy dane są aktualne (otrzymane w ciągu ostatnich 5 sekund)
  if (millis() - lastSpeedDataTime > 5000) {
    html += "<p class='value stale'>Data connection lost</p>";
  }
  html += "</div>";
  
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
  
  // Dodaj funkcje do aktualizacji telemetrii
  html += "function updateTelemetry(){";
  html += "  fetch('/telemetry_data').then(response=>response.json()).then(data=>{";
  html += "    document.getElementById('speed').textContent=data.speed.toFixed(1);";
  html += "    document.getElementById('distance').textContent=data.distance.toFixed(1);";
  html += "    if(data.stale){";
  html += "      document.querySelector('.telemetry').classList.add('stale');";
  html += "    }else{";
  html += "      document.querySelector('.telemetry').classList.remove('stale');";
  html += "    }";
  html += "  });";
  html += "}";

  html += "setInterval(updateImage,100);"; // Częstsze odświeżanie - co 100ms
  html += "setInterval(updateTelemetry,1000);"; // Aktualizacja telemetrii co sekundę
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

// Funkcja do obsługi endpointu /speed_data
void handleSpeedData() {
  if (server.hasArg("speed") && server.hasArg("distance")) {
    carSpeed = server.arg("speed").toFloat();
    carDistance = server.arg("distance").toFloat();
    lastSpeedDataTime = millis();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// Funkcja do dostarczania danych telemetrycznych w formacie JSON
void handleTelemetryData() {
  String json = "{";
  json += "\"speed\":" + String(carSpeed, 2) + ",";
  json += "\"distance\":" + String(carDistance, 2) + ",";
  json += "\"stale\":" + String((millis() - lastSpeedDataTime > 5000) ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-CAM inicjalizacja");
  
  // Inicjalizacja kamery
  initCamera();
  
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // Wyłącz tryb oszczędzania energii WiFi
  
  // Konfiguracja statycznego IP bazująca na wykrytych wartościach
  IPAddress staticIP(192, 168, 251, 156);  // Przypisany adres IP z poprzedniej sesji
  IPAddress gateway(192, 168, 251, 164);   // Adres bramy (hotspot telefonu)
  IPAddress subnet(255, 255, 255, 0);      // Maska podsieci
  IPAddress dns(192, 168, 251, 164);       // Serwer DNS (ten sam co brama)
  
  // Zastosuj konfigurację statycznego IP
  if (!WiFi.config(staticIP, gateway, subnet, dns)) {
    Serial.println("Błąd konfiguracji statycznego IP - używam DHCP");
  } else {
    Serial.println("Zastosowano konfigurację statycznego IP");
  }
  
  // Rozpocznij łączenie z siecią
  Serial.printf("Łączenie z Wi-Fi: %s\n", ssid);
  WiFi.begin(ssid, password);
  
  // Dodajemy timeout dla połączenia WiFi
  unsigned long startAttemptTime = millis();
  
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    delay(500);
    Serial.print(".");
  }
  
  // Sprawdź czy połączono
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nBłąd połączenia z WiFi. Resetowanie...");
    delay(1000);
    ESP.restart();
    return;
  }

  Serial.println("");
  Serial.println("Połączono z Wi-Fi");
  Serial.print("Adres IP: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.on("/stream", handleStream);
  server.on("/speed_data", HTTP_GET, handleSpeedData);
  server.on("/telemetry_data", HTTP_GET, handleTelemetryData);
  server.begin();
  Serial.println("Serwer HTTP uruchomiony");
}

void loop() {
  server.handleClient();
  delay(1); // Minimalne opóźnienie dla stabilności
}
