/******************************************************************************
 CameraWebServerFTP TimerCAM forked by bokunimo.net
 ******************************************************************************
 Wi-Fi カメラ M5 Timer CAM が顔を検知した時や人感センサ（PIR Unit）が人体などの
 動きを検知したとき、あるいは設定した周期ごとに、写真を撮影し、FTP サーバや、
 LINE、Windows PC、Raspberry Pi 等へ転送します。
                                                                 Wataru KUNINO
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/*  M5Stack Timer CAM 専用 */
///////////////////////////////////////////////////////////////////////////////

// Arduino IDEの ボード設定で M5Stack-Timer-CAM を選択してください。
// 後述の「コンパイル方法」を参照のこと。

#define CAMERA_MODEL_M5STACK_TimerCAM


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
 ******************************************************************************
 バッテリ(M5Stack Battery Support Base 400mAh)動作時の間欠動作用に設定します。
 約10分間隔のときの動作時間(実測)：19.5時間（10/09 12:45:59～10/10 08:18:50）
 ※ PWDN_GPIO_NUM<0 のデバイスはカメラをOFFできない。
 ※ 転送画像が途中で切れる時は スリープ待機 SLEEP_WAIT を増やしてください。
 *****************************************************************************/
// SLEEP_P 0ul                              // 無効
// SLEEP_P 16*1000000ul                     // スリープ時間 16秒
// SLEEP_P 56*1000000ul                     // スリープ時間 56秒
// SLEEP_P 296*1000000ul                    // スリープ時間 約5分(296秒)
// SLEEP_P 596*1000000ul                    // スリープ時間 約10分(596秒)
// SLEEP_P 1796*1000000ul                   // スリープ時間 約30分(1796秒)
// SLEEP_P 3596*1000000ul                   // スリープ時間 約60分(3596秒)

#define SLEEP_P 0ul                         // 無効
#define SLEEP_WAIT 1.5                      // スリープ待機
/******************************************************************************
 コンパイル方法
 ******************************************************************************
 Arduino IDEと arduino-esp32 をインストールし、組み込んでコンパイルを行います。
 arduino-esp32のバージョンは 2.0.0-alpha1 を使用して動作確認しました。

 必要なライブラリ：
 arduino-esp32：https://github.com/espressif/arduino-esp32/releases
 
 コンパイル時の設定：
 [M5Stack-Timer-CAM]を選択してください
  ・Arduino IDE：[ツール]⇒[ボード]⇒[M5Stack-Timer-CAM]
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
    - wakeup.ino 
        MIT License (ファイル=wakeup_LICENSE)
        https://github.com/m5stack/TimerCam-arduino
    - battery.c battery.h
        MIT License (ファイル=battery_LICENSE)
        https://github.com/m5stack/TimerCam-arduino
    - bmm8563.c bmm8563.h
        MIT License (ファイル=bmm8563_LICENSE)
        https://github.com/m5stack/TimerCam-arduino
    - led.c led.h
        MIT License (ファイル=led_LICENSE)
        https://github.com/m5stack/TimerCam-arduino
    - 改変部の著作権は Wataru KUNINO (bokunimo.net) が所有します。
 *****************************************************************************/

/******************************************************************************
 プログラム部
 *****************************************************************************/

#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "battery.h"
#include "led.h"
#include "bmm8563.h"

#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_camera.h"
#include "./camera_pins.h"                  // 選択したカメラの設定値の組込部
#include "./app_httpd.h"                    // カメラ制御用ＩＦ部の組み込み

char        CONFIGFILE[] = "/camset.txt";   // 設定ファイルの保存名(SPIFFS)
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
int         wake;                           // 起動モード(TimerWakeUp受取用)
int         pir;                            // 人感センサ値
int         send_c = 0;                     // ソフト・タイマー用カウンタ
int         wifi_mode;                      // Wi-Fiモード 0:親機AP 1:子機STA
byte        MAC[6];                         // 本機MACアドレス
unsigned long TIME = 0;                     // タイマー用変数
char cc_date[] = __DATE__ ;
char cc_time[] = __TIME__ ;

void sendUdp(String dev, String S){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    udp.beginPacket(IP_BROAD, PORT);        // UDP送信先を設定
    udp.println(dev + S);
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    Serial.println("udp://" + IP_BROAD.toString() + ":" + PORT + " " + dev + S);
    delay(100);                             // 送信待ち時間
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

int keepalive = 0;                          // deepsleep待機延長時間 1/10秒単位
void deepsleep_keepalive(int sec){
    // Serial.printf("\n keepa=%.1f, sec=%d ---\n",-(float)keepalive/10,sec);
    if(keepalive > -sec * 10) keepalive = -sec * 10;
}

void deepsleep(uint32_t us){
    Serial.printf("Going to sleep for %d seconds in %.1f seconds\n",(int)(us / 1000000ul),SLEEP_WAIT);
    // if(PWDN_GPIO_NUM < 0) Serial.println(",but the camera module will be eating.");
    delay((int)(SLEEP_WAIT * 1000));
    Serial.println("zzz...");
    delay(102);

    sleepTimerCAM(us);

    // esp_camera_deinit();
    // esp_deep_sleep(us);                     // Deep Sleepモードへ移行

    while(1) delay(100);
}

void setup() {
    esp_efuse_mac_get_default(MAC);
    pinMode(0, INPUT_PULLUP);
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println("\nM5Camera started");
    Serial.printf("MAC Address = %02x:%02x:%02x:%02x:%02x:%02x\n",MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
    wake = TimerWakeUp_init();
    #ifdef CAMERA_MODEL_M5STACK_TimerCAM
        boolean ret = setupTimerCAM(!wake);	// wakeが0のときに時刻をセット
        if(wake == 0) wake = rtc_wakeup_reason();
    #endif
    if(wake == 0) deepsleep_keepalive(20);

    // LED ON
    if(LED_GPIO_NUM){
        pinMode(LED_GPIO_NUM, OUTPUT);
        digitalWrite(LED_GPIO_NUM, LOW);
    }
    
    // 人感センサ
    if(PIR_GPIO_NUM > 0){
        pinMode(PIR_GPIO_NUM, INPUT_PULLUP);
        Serial.printf("PIR = %d, ", digitalRead(PIR_GPIO_NUM));
        if(SLEEP_P == 0) delay(100);
        Serial.printf("%d, ", digitalRead(PIR_GPIO_NUM));
        if(SLEEP_P == 0) delay(2000 * digitalRead(PIR_GPIO_NUM));
        pir = digitalRead(PIR_GPIO_NUM);
        Serial.printf("%d\n", pir);
    }
    
    // SPIFFS
    if( ( !SPIFFS.begin() || !digitalRead(0) ) && wake == 0 ){  // ファイルシステムSPIFFSの開始
        Serial.println("Formating SPIFFS.");
        SPIFFS.format();
        SPIFFS.begin(); // エラー時にSPIFFSを初期化
    }
    
    cameraMyConfig();

    Serial.println("Starting Wi-Fi");
    delay(10);

    WiFi.mode(WIFI_STA);
    wifi_mode = 1;
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    TIME = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        if(LED_GPIO_NUM) digitalWrite(LED_GPIO_NUM, !digitalRead(LED_GPIO_NUM));
        Serial.print(".");
        if(millis() - TIME > TIMEOUT){
            // AP起動条件：SSIDが初期値 かつ 電源orリセット起動時(割り込み起動時除く)
            if(strcmp("1234ABCD",WIFI_SSID) == 0 || wake == 0){
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
    else                                     line_sender_enabled = 0;
    if(strcmp("your_password",FTP_PASS) != 0) ftp_sender_enabled = 1;
    else                                      ftp_sender_enabled = 0;
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
        "%04d/%04d, pir:%d,(%d), face:%d,%d,(%d), udp:%d, ftp:%d, line:%d",
        send_c,send_interval,
        pir_enabled,(pir),
        face_detection_enabled,face_recognition_enabled,(face),
        udp_sender_enabled,ftp_sender_enabled,line_sender_enabled
    );
    if(LED_GPIO_NUM) digitalWrite(LED_GPIO_NUM, !send_c);
    if(wake) Serial.printf(", wake:%d", wake);
    if(!wifi_mode) Serial.printf(", ap_mode:%d", (int)(TIME-millis())/1000+600);
    Serial.println();
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

    int waiting = 1000 - (send_c ? 0 : 100) - (face ? 200 : 0);
    for(; waiting > 0; waiting -= 100){
        Serial.print('.');
        if(LED_GPIO_NUM && !wifi_mode) digitalWrite(LED_GPIO_NUM, !digitalRead(LED_GPIO_NUM));
        for(int i=0;i<100;i++) delay(1);
    }
    if(send_interval) send_c++; else send_c = 1;
    if(!wifi_mode && millis() - TIME < 600000ul) deepsleep(600*1000000ul);
                                        // 親機かつ、起動後600秒(10分)以内のとき
    if(!SLEEP_P && wifi_mode) return;   // deepsleep未設定 かつ 子機STAのとき
    
    // 間欠送信処理中の DEEP SLEEP 処理 ////////////////////////////////////////////////////
    uint32_t us = SLEEP_P;
    if(!us) us = 600*1000000ul;         // 10分間
    Serial.printf("Going to sleep for %d seconds in %.1f seconds\n"
        ,(int)(us / 1000000ul),SLEEP_WAIT);
    // if(PWDN_GPIO_NUM < 0) Serial.println(",but the camera module will be eating.");
    for(;keepalive < (int)(SLEEP_WAIT * 10); keepalive++){
        Serial.print('.');
        for(int i=0;i<100;i++) delay(1);
        if(LED_GPIO_NUM) digitalWrite(LED_GPIO_NUM, keepalive % 2);
        if(keepalive % 10 == 0){
            Serial.printf(" in %d seconds\n",((int)(SLEEP_WAIT * 10) - keepalive) / 10 + 1);
        }
    }
    if(LED_GPIO_NUM) digitalWrite(LED_GPIO_NUM, HIGH);
    uint32_t ms1 = millis();
    uint32_t ms2 = ms1 + us / 1000 + 6;
    Serial.printf("zzz...\nDuty Cycle = %.1f(s) / %.1f(s) = 1 / %.1f\n\n",
        (float)ms1/1000., (float)ms2/1000., (float)ms2 / (float)ms1);
    delay(6);                               // シリアル待ち時間
    
    sleepTimerCAM(SLEEP_P);

    // esp_camera_deinit();
    // esp_deep_sleep(us);                     // Deep Sleepモードへ移行

    while(1) delay(100);
}
