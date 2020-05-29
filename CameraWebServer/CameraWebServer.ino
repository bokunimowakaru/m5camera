#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>                    // UDP通信を行うライブラリ

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE	// Has PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
//#define CAMERA_MODEL_TTGO_T_CAMERA
#define CAMERA_MODEL_TTGO_T_CAMERA_V16

#include "camera_pins.h"


/******************************************************************************
 *  WiFi 下記にゲートウェイのWi-Fi設定を入力してください
 *****************************************************************************/
#define WIFI_SSID   "1234ABCD"          // your wifi ssid
#define WIFI_PASSWD "password"          // your wifi password

#define SENDTO "255.255.255.255"        // UDP送信先IPアドレス
#define PORT 1024                       // UDP送信先ポート番号
#define DEVICE_CAM "cam_a_5,"           // デバイス名(カメラ)

void startCameraServer();
int8_t udp_sender_enabled = 1;
int8_t ftp_sender_enabled = 0;
int16_t send_interval = 60;
String ip;

void sendUdp_Ident(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    String S = String(DEVICE_CAM) + String(0) + ", http://" + ip + "/cam.jpg";
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.println(S);
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    Serial.println(S);
    delay(200);                             // 送信待ち時間
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("M5Camera started");

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
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE) || defined(CAMERA_MODEL_TTGO_T_CAMERA_V16)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  Serial.println("OV2640 started");

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_TTGO_T_CAMERA_V16) || defined(CAMERA_MODEL_TTGO_T_CAMERA_V16)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  Serial.println("Starting Wi-Fi");
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  ip = WiFi.localIP().toString();
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(ip);
  Serial.println("' to connect");
  sendUdp_Ident();
}

int send_c = 0;
void loop() {
  Serial.printf("count:%d, udp:%d, ftp:%d, interval:%d\n",send_c,udp_sender_enabled,ftp_sender_enabled,send_interval);
  if(send_c >= send_interval){
    sendUdp_Ident();
    send_c = 0;
  }
  delay(1000);
  send_c++;
}
