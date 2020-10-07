/******************************************************************************
 CameraWebServerFTP forked by bokunimo.net
 ******************************************************************************
 Wi-Fi 搭載カメラ M5Camera が顔を検知した時や人感センサ（PIR Unit）が人体などの
 動きを検知したとき、あるいは設定した周期ごとに、写真を撮影し、FTP サーバや、
 LINE、Windows PC、Raspberry Pi 等へ転送します。
                                                                 Wataru KUNINO
 *****************************************************************************/

/******************************************************************************
 カメラ 選択 ※使用するカメラを選択してください。
 ******************************************************************************
 ※ M5Camera version B を使用する場合は変更不要です。
 ※ version A の場合は #define CAMERA_MODEL_M5STACK_PSRAMに変更してください。
 *****************************************************************************/
// CAMERA_MODEL_M5STACK_PSRAM               // M5Camera version A
// CAMERA_MODEL_M5STACK_V2_PSRAM            // M5Camera version B
// CAMERA_MODEL_TTGO_T_CAMERA               // TTGO T-Camera OV2640_V05用
// CAMERA_MODEL_TTGO_T_CAMERA_V16           // TTGO T-Camera OV2640_V1.6用

#define CAMERA_MODEL_M5STACK_V2_PSRAM       // [選択例] M5Camera version B

/******************************************************************************
 Wi-Fi 設定 ※下記にゲートウェイのWi-Fi設定を入力してください
 *****************************************************************************/
#define WIFI_SSID   "1234ABCD"              // your wifi ssid (★要設定)
#define WIFI_PASSWD "password"              // your wifi password (★要設定)
#define TIMEOUT 7000                        // Wi-Fi接続タイムアウト 7秒

/******************************************************************************
 FTP 設定
 ******************************************************************************
 ※FTPサーバが必要です。
 　ラズベリーパイへ、FTPサーバをセットアップするには下記を実行してください。
 　~/m5camera/tools/ftp_setup.sh
 ※FTPでは、ユーザ名やパスワード、データーが平文で転送されます。
 　インターネット上で扱う場合は、セキュリティに対する配慮が必要です。
 ※下記define文にFTPパスワードを設定すると、自動的に FTP = ON になります。
 ・ユーザ画面(HTMLコンテンツ)の初期値は FTP = OFF のままです。
 　気になる方は、HTMLファイルを書き換えてコンパイルしなおしてください。
 *****************************************************************************/
#define FTP_TO   "192.168.4.2"              // FTP 送信先のIPアドレス(★要設定)
#define FTP_USER "pi"                       // FTP ユーザ名(★要設定)
#define FTP_PASS "your_password"            // FTP パスワード(★要設定)
#define FTP_DIR  "~/"                       // FTP ディレクトリ(Raspberry Pi等)
#define Filename "cam_a_5_0000.jpg"         // FTP 保存先のファイル名

/******************************************************************************
 LINE Notify 設定
 ******************************************************************************
 ※LINE アカウントと LINE Notify 用のトークンが必要です。
    1. https://notify-bot.line.me/ へアクセス
    2. 右上のアカウントメニューから「マイページ」を選択
    3. トークン名「esp32」を入力
    4. 送信先のトークルームを選択する（「1:1でLINE Notifyから通知を受け取る」など）
    5. [発行する]ボタンでトークンが発行される
    6. [コピー]ボタンでクリップボードへコピー
    7. 下記のLINE_TOKENに貼り付け
 *****************************************************************************/
#define LINE_TOKEN  "your_token"            // LINE Notify 用トークン(★要設定)
#define MESSAGE_PIR "人感センサが反応しました。"
#define MESSAGE_CAM "カメラが顔を検知しました。"

/******************************************************************************
 UDP 設定
 *****************************************************************************/
#define PORT 1024                           // UDP送信先ポート番号
#define DEVICE_CAM  "cam_a_5,"              // デバイス名(カメラ)
#define DEVICE_PIR  "pir_s_5,"              // デバイス名(人感センサ)

/******************************************************************************
 DEEP SLEEP 設定
 *****************************************************************************/
// SLEEP_P 0ul                              // 無効
// SLEEP_P 50*1000000ul                     // スリープ時間 50秒
// SLEEP_P 290*1000000ul                    // スリープ時間 約5分(290秒)
// SLEEP_P 590*1000000ul                    // スリープ時間 約10分(590秒)
// SLEEP_P 1790*1000000ul                   // スリープ時間 約30分(1790秒)
// SLEEP_P 3590*1000000ul                   // スリープ時間 約60分(3590秒)
#define SLEEP_P    0ul                      // 無効
#define SLEEP_WAIT 0                        // スリープ遅延 0秒

/******************************************************************************
 コンパイル方法
 ******************************************************************************
 Arduino IDEと arduino-esp32 をインストールし、組み込んでコンパイルを行います。
 arduino-esp32のバージョンは 1.0.4 を使用しました（1.2.0未満では動作しない）。

 必要なライブラリ：
 arduino-esp32：https://github.com/espressif/arduino-esp32/releases
 
 コンパイル時の設定：
 [ESP32 Wrover Module]と[Huge App]を選択してください
  ・Arduino IDE：[ツール]⇒[ボード]⇒[ESP32 Wrover Module]
  ・Partition Scheme : [Huge App (3MB APP No OTA/1MB SPIFFS)]
  ・PSRAM : Enable
 *****************************************************************************/

/******************************************************************************
 権利情報：このソースコード等には、下記の製作物が含まれます。
    - Arduino core for the ESP32：
        Espressif Systems Shanghai,China http://www.espressif.com
        GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
    - CameraWebServer (ESP32 sketches examples)：
        Hristo Gochkov, Ivan Grokhtkov
        GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
    - app_httpd.cpp：
        Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
        Apache License, Version 2.0
    - 改変部の著作権は Wataru KUNINO (bokunimo.net) が所有します。
 *****************************************************************************/

/******************************************************************************
 プログラム部
 *****************************************************************************/

#include <WiFi.h>
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#include "esp_camera.h"
#include "camera_pins.h"                    // 選択したカメラの設定値の組込部
#include "app_httpd.h"                      // カメラ制御用ＩＦ部の組み込み

int         FileNumMax = 16;                // FTP 保存先の最大ファイル数
int         FileNum = 1;                    // 現在のファイル番号
boolean     FileNumOVF = false;             // オーバーフロー用フラグ
int8_t      face_detection_enabled = 0;     // 顔検知 無効=0,有効=1
int8_t      face_recognition_enabled = 0;   // 顔認証 無効=0,有効=1
int8_t      pir_enabled = 1;                // PIR 人感センサ無効=0,有効=1
int8_t      udp_sender_enabled = 1;         // UDP 送信無効=0,有効=1
int8_t      ftp_sender_enabled = 0;         // FTP 送信無効=0,有効=1
int8_t      line_sender_enabled = 1;        // LINE HTTP 送信無効=0,有効=1
int16_t     send_interval = 30;             // 送信間隔（秒）
IPAddress   IP_LOCAL;                       // 本機のIPアドレス
IPAddress   IP_BROAD;                       // ブロードキャストIPアドレス
int         pir;                            // 人感センサ値
int         send_c = 0;                     // ソフト・タイマー用カウンタ
int         wifi_mode;                      // Wi-Fiモード 0:親機AP 1:子機STA
byte        MAC[6];                         // 本機MACアドレス
unsigned long TIME = 0;                     // タイマー用変数

void sendUdp(String dev, String S){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    udp.beginPacket(IP_BROAD, PORT);        // UDP送信先を設定
    udp.println(dev + S);
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    Serial.println("udp://" + IP_BROAD.toString() + ":" + PORT + " " + dev + S);
    delay(200);                             // 送信待ち時間
}

void sendUdp_Fd(uint16_t fd_num){
    sendUdp(DEVICE_CAM, String(fd_num) + ", http://" + IP_LOCAL.toString() + "/cam.jpg");
}
void sendUdp_Pir(int pir){
    sendUdp(DEVICE_PIR, String(pir));
}

void sendUdp_Ident(){
    sendUdp_Fd(0);
}

void deepsleep(uint32_t us){
    Serial.printf("Going to sleep for %d seconds in %d seconds\n",(int)(us / 1000000ul),SLEEP_WAIT);
    for(int i = SLEEP_WAIT; i >= 0 ; i--){  // HTTPアクセス待ち時間
        for(int j=0;j<10;j++){
            delay(100);                     // 100ms * 10 = 1秒の待ち時間
            if(LED_GPIO_NUM) digitalWrite(LED_GPIO_NUM, j % 2);
        }
        Serial.println(i);
    }
    if(LED_GPIO_NUM) digitalWrite(LED_GPIO_NUM, HIGH);
    Serial.println("zzz...");
    delay(102);                             // 送信待ち時間
    esp_deep_sleep(us);                     // Deep Sleepモードへ移行
    while(1) delay(100);
}

void setup() {
    esp_efuse_mac_get_default(MAC);
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println("\nM5Camera started");
    Serial.printf("MAC Address = %02x:%02x:%02x:%02x:%02x:%02x\n",MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);

    // LED ON
    if(LED_GPIO_NUM){
        pinMode(LED_GPIO_NUM, OUTPUT);
        digitalWrite(LED_GPIO_NUM, LOW);
    }
    
    // 人感センサ
    if(PIR_GPIO_NUM > 0){
        pinMode(PIR_GPIO_NUM, INPUT_PULLUP);
        Serial.printf("PIR = %d, ", digitalRead(PIR_GPIO_NUM));
        delay(100);
        Serial.printf("%d, ", digitalRead(PIR_GPIO_NUM));
        if(SLEEP_P == 0) delay(2000 * digitalRead(PIR_GPIO_NUM));
        pir = digitalRead(PIR_GPIO_NUM);
        Serial.printf("%d\n", pir);
    }
    
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
    
    // WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
    //            Ensure ESP32 Wrover Module or other board with PSRAM is selected
    //            Partial images will be transmitted if image exceeds buffer size
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

#if defined(CAMERA_MODEL_M5STACK_WIDE) || \
    defined(CAMERA_MODEL_TTGO_T_CAMERA) || \
    defined(CAMERA_MODEL_M5STACK_V2_PSRAM) || \
    defined(CAMERA_MODEL_TTGO_T_CAMERA_V16)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
#endif

    Serial.println("Starting Wi-Fi");
    delay(100);

    WiFi.mode(WIFI_STA);
    wifi_mode = 1;
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    TIME=millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(200);
        if(LED_GPIO_NUM) digitalWrite(LED_GPIO_NUM, !digitalRead(LED_GPIO_NUM));
        Serial.print(".");
        if(millis()-TIME>TIMEOUT){
            if(strcmp("1234ABCD",WIFI_SSID) == 0){
                WiFi.disconnect();              // WiFiアクセスポイントを切断する
                Serial.println("\nWi-Fi AP Mode");// 接続が出来なかったときの表示
                char s[22];
                sprintf(s,"M5CAM_%02X%02X",MAC[4],MAC[5]);
                WiFi.mode(WIFI_AP); delay(100); // 無線LANを【AP】モードに設定
                WiFi.softAP(s);                 // ソフトウェアAPの起動
                WiFi.softAPConfig(
                    IPAddress(192,168,4,1),     // AP側の固定IPアドレスの設定
                    IPAddress(0,0,0,0),         // 本機のゲートウェイアドレスの設定
                    IPAddress(255,255,255,0)    // ネットマスクの設定
                );
                wifi_mode = 0;
            }else if(SLEEP_P != 0) deepsleep(SLEEP_P);
            else deepsleep(10 * 1000000ul);
            break;
        }
    }
    if(wifi_mode == 1) IP_LOCAL = WiFi.localIP();
    else               IP_LOCAL = WiFi.softAPIP();
    Serial.println(IP_LOCAL);
    Serial.println("WiFi connected");

    startCameraServer();
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(IP_LOCAL);
    Serial.println("' to connect");

    IP_BROAD = IP_LOCAL;
    IP_BROAD[3] = 255;
    sendUdp_Ident();
    
    if(strcmp("your_token",LINE_TOKEN) != 0) line_sender_enabled = 1;
    if(strcmp("your_password",FTP_PASS) != 0) ftp_sender_enabled = 1;
}

void loop() {
    uint16_t face = get_face_detected_num();
    if(LED_GPIO_NUM) digitalWrite(LED_GPIO_NUM, 0);
    
    /* トリガ①：カメラによる顔検出 */
    if(face_detection_enabled || face_recognition_enabled){
        if(!face){
            capture_face();
            face = get_face_detected_num();
        }
        if(face){
            Serial.println("[!] Triggered by Face Detector");
            if(udp_sender_enabled) sendUdp_Fd(face);
            if(line_sender_enabled) LINE_Camera(LINE_TOKEN, MESSAGE_CAM);
            send_c = 0;
        }
    }
    
    /* トリガ②：人感センサ（PIR Unit）による動体検出 */
    if(pir_enabled && PIR_GPIO_NUM > 0 && pir != digitalRead(PIR_GPIO_NUM) ){
        pir = !pir;
        if(pir){
            Serial.println("[!] Triggered by PIR");
            if(line_sender_enabled) LINE_Camera(LINE_TOKEN, MESSAGE_PIR);
            send_c = 0;
        }
        if(udp_sender_enabled) sendUdp_Pir(pir);
        // M5 STACK PIR UNIT の信号保持時間は2秒なので1秒毎の確認でOK
    }
        
    /* トリガ③：インターバル・タイマ設定時間の経過 */
    if(send_interval && send_c >= send_interval){
        Serial.println("[!] Triggered by Interval Timer");
        if(udp_sender_enabled) sendUdp_Ident();
        // if(line_sender_enabled) LINE_Camera(LINE_TOKEN, MESSAGE_CAM);
        send_c = 0;
    }
    
    /* 状態をシリアル出力 */
    Serial.printf(
        "%04d/%04d, pir:%d,(%d), face:%d,%d,(%d), udp:%d, ftp:%d, line:%d\n",
        send_c,send_interval,
        pir_enabled,(pir),
        face_detection_enabled,face_recognition_enabled,(face),
        udp_sender_enabled,ftp_sender_enabled,line_sender_enabled
    );
    if(LED_GPIO_NUM) digitalWrite(LED_GPIO_NUM, !send_c);
    
    /* FTP送信（トリガ発生時） */
    if(ftp_sender_enabled && !send_c ){
        int len = strlen(Filename);
        if(len <= 16){
            char s[17] = Filename;
            int i, div = 1;
            for(i=0; i<4; i++){
                s[len - 5 - i] = '0' + ((FileNum / div) % 10);
                div *= 10;
            }
            
            i = FTP_Camera(s);
            if(i) Serial.printf("ERROR: FTP (%d)\n",i);
            
            i = FTP_Html(FileNum, FileNumOVF);
            if(i) Serial.printf("ERROR: FTP (%d)\n",i);
            
            FileNum++;
            if(FileNum > FileNumMax){
                FileNum = 0;        // FileNumMaxよりも1だけ多く(書き換え用)
                FileNumOVF = true;  // 1周した
            }
        } else Serial.printf("ERROR: length of Filename %d\n",len);
    }

    delay(1000 - (send_c ? 0 : 200) - (face ? 200 : 0));
    if(send_interval) send_c++; else send_c = 1;
    if(SLEEP_P != 0) deepsleep(SLEEP_P);
}
