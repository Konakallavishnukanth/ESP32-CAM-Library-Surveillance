#include <WiFi.h> 
#include <WebServer.h> 
#include <WiFiClient.h> 
#include <esp_camera.h> 
#include <WiFiManager.h> // WiFiManager by tzapu 
#include <set> 
 
// ESP32-CAM (AI Thinker) pin configuration 
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
 
#define FLASH_GPIO_NUM     4  // Built-in flash LED pin 
 
WebServer server(80); 
 
// Hardcoded login credentials 
const char* loginUsername = "admin"; 
const char* loginPassword = "password"; 
 
// Track login state by IP 
std::set<IPAddress> loggedInClients; 
 
bool isClientLoggedIn() { 
  return loggedInClients.find(server.client().remoteIP()) != 
loggedInClients.end(); 
} 
 
// Login HTML page 
const char* loginPage = R"rawliteral( 
<!DOCTYPE html> 
<html> 
<head> 
  <title>Login - ESP32-CAM</title> 
  <style> 
    body { 
      font-family: Arial, sans-serif; 
      background-image: url('https://www.pixelstalk.net/wp
content/uploads/2016/08/Beautiful-Library-Wallpaper.jpg'); 
      background-size: cover; 
      margin: 0; 
      display: flex; 
      flex-direction: column; 
      align-items: center; 
      justify-content: center; 
      height: 100vh; 
      color: white; 
    } 
    .navbar { 
      background-color: #d17b4f; 
      color: white; 
      padding: 15px; 
      width: 100%; 
      text-align: center; 
      font-size: 24px; 
      font-weight: bold; 
      position: absolute; 
      top: 0; 
    } 
    .login-box { 
      background-color: rgba(0, 0, 0, 0.7); 
      padding: 30px; 
      border-radius: 10px; 
      margin-top: 80px; 
    } 
    input { 
      width: 100%; 
      padding: 10px; 
      margin: 8px 0; 
      border-radius: 5px; 
      border: none; 
    } 
    button { 
      padding: 10px 20px; 
      background-color: #d17b4f; 
      border: none; 
      color: white; 
      font-size: 16px; 
      border-radius: 5px; 
      cursor: pointer; 
    } 
  </style> 
</head> 
<body> 
  <div class="navbar">ESP32-CAM Login</div> 
  <div class="login-box"> 
    <form method="POST" action="/login"> 
      <label>Username:</label><br> 
      <input type="text" name="username" required><br> 
      <label>Password:</label><br> 
      <input type="password" name="password" required><br><br> 
      <button type="submit">Login</button> 
    </form> 
  </div> 
</body> 
</html> 
)rawliteral"; 
 
// HTML with stream and snapshot 
const char* htmlHomePage = R"rawliteral( 
<!DOCTYPE html> 
<html> 
<head> 
  <title>ECE Department Library - ESP32-CAM</title> 
  <style> 
    body { 
      font-family: Arial, sans-serif; 
      margin: 0; 
      background-image: url('https://www.pixelstalk.net/wp
content/uploads/2016/08/Beautiful-Library-Wallpaper.jpg'); 
      background-size: cover; 
      color: #333; 
    } 
    .navbar { 
      background-color: #d17b4f; 
      color: white; 
      padding: 15px; 
      text-align: center; 
      font-size: 24px; 
      font-weight: bold; 
    } 
    .content { 
      display: flex; 
      justify-content: center; 
      align-items: center; 
      margin-top: 30px; 
      flex-direction: column; 
    } 
    .video-container { 
      border: 4px solid #d17b4f; 
      padding: 10px; 
      background: white; 
      box-shadow: 0 0 15px rgba(0,0,0,0.1); 
      width: 90vw; 
      max-width: 1280px; 
    } 
    img { 
      width: 100%; 
      height: auto; 
      display: block; 
    } 
    button { 
      margin-top: 20px; 
      padding: 12px 20px; 
      font-size: 16px; 
      background-color: #d17b4f; 
      color: white; 
      border: none; 
      border-radius: 6px; 
      cursor: pointer; 
    } 
  </style> 
</head> 
<body> 
  <div class="navbar">ECE Department Library - ESP32-CAM</div> 
  <div class="content"> 
    <div class="video-container"> 
      <img src="/stream" /> 
    </div> 
    <button onclick="captureSnapshot()">         Capture Snapshot</button> 
  </div> 
  <script> 
    function captureSnapshot() { 
      const link = document.createElement('a'); 
      link.href = '/capture?' + new Date().getTime(); // prevent cache 
      link.download = 'snapshot.jpg'; 
      document.body.appendChild(link); 
      link.click(); 
      document.body.removeChild(link); 
    } 
  </script> 
</body> 
</html> 
)rawliteral"; 
 
void handle_jpg_stream() { 
  WiFiClient client = server.client(); 
  String response = "HTTP/1.1 200 OK\r\n"; 
  response += "Content-Type: multipart/x-mixed-replace; 
boundary=frame\r\n\r\n"; 
  client.print(response); 
 
  while (client.connected()) { 
    camera_fb_t *fb = esp_camera_fb_get(); 
    if (!fb) { 
      Serial.println("Camera capture failed"); 
      return; 
    } 
 
    client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: 
%u\r\n\r\n", fb->len); 
    client.write(fb->buf, fb->len); 
    client.print("\r\n"); 
 
    esp_camera_fb_return(fb); 
    delay(50); 
  } 
} 
 
void setup() { 
  Serial.begin(115200); 
  delay(1000); 
 
  pinMode(FLASH_GPIO_NUM, OUTPUT); 
  digitalWrite(FLASH_GPIO_NUM, HIGH); 
 
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
  config.jpeg_quality = 12; 
  config.fb_count = 1; 
 
  if (esp_camera_init(&config) != ESP_OK) { 
    Serial.println("Camera init failed"); 
    return; 
  } 
 
  WiFiManager wm; 
  wm.setConnectTimeout(30); 
  if (!wm.autoConnect("ESP32-CAM-Setup")) { 
    Serial.println("Failed to connect. Restarting..."); 
    ESP.restart(); 
  } 
 
  Serial.println("WiFi connected!"); 
  Serial.print("IP address: "); 
  Serial.println(WiFi.localIP()); 
 
  server.on("/", []() { 
    if (!isClientLoggedIn()) { 
      server.sendHeader("Location", "/login"); 
      server.send(303); 
      return; 
    } 
    server.send(200, "text/html", htmlHomePage); 
  }); 
 
  server.on("/login", HTTP_GET, []() { 
    server.send(200, "text/html", loginPage); 
  }); 
 
  server.on("/login", HTTP_POST, []() { 
    if (server.arg("username") == loginUsername && server.arg("password") == 
loginPassword) { 
      loggedInClients.insert(server.client().remoteIP()); 
      server.sendHeader("Location", "/"); 
      server.send(303); 
    } else { 
      server.send(200, "text/html", "<h2>Login Failed</h2><a href='/login'>Try 
Again</a>"); 
    } 
  }); 
 
  server.on("/stream", HTTP_GET, []() { 
    if (!isClientLoggedIn()) { 
      server.sendHeader("Location", "/login"); 
      server.send(303); 
      return; 
    } 
    handle_jpg_stream(); 
  }); 
 
  server.on("/capture", HTTP_GET, []() { 
    if (!isClientLoggedIn()) { 
      server.sendHeader("Location", "/login"); 
      server.send(303); 
      return; 
    } 
    camera_fb_t *fb = esp_camera_fb_get(); 
    if (!fb) { 
      server.send(500, "text/plain", "Camera capture failed"); 
      return; 
    } 
    server.sendHeader("Content-Type", "image/jpeg"); 
    server.sendHeader("Content-Length", String(fb->len)); 
    server.send(200, "image/jpeg", fb->buf, fb->len); 
    esp_camera_fb_return(fb); 
  }); 
 
  server.begin(); 
} 
 
void loop() { 
  server.handleClient(); 
} 
