/*******************************************************************************
LINE送信用 HTTPクライアント

1. https://notify-bot.line.me/ へアクセス
2. 右上のアカウントメニューから「マイページ」を選択
3. トークン名「esp32」を入力
4. 送信先のトークルームを選択する（「1:1でLINE NOtifyから通知を受け取る」など）
5. [発行する]ボタンでトークンが発行される
6. [コピー]ボタンでクリップボードへコピー

curl -X POST -H "Authorization: Bearer token" -F "message=hello" https://notify-api.line.me/api/notify
*******************************************************************************/

/******************************************************************************
 参考資料 1 WifiClientSecure
 ******************************************************************************
  Wifi secure connection example for ESP32
  Running on TLS 1.2 using mbedTLS
  2017 - Evandro Copercini - Apache 2.0 License.
 ******************************************************************************/

/******************************************************************************
 参考資料 2 CameraWebServer app_httpd.cpp
 ******************************************************************************
    // Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
    //
    // Licensed under the Apache License, Version 2.0 (the "License");
    // you may not use this file except in compliance with the License.
    // You may obtain a copy of the License at
    //
    //     http://www.apache.org/licenses/LICENSE-2.0
    //
    // Unless required by applicable law or agreed to in writing, software
    // distributed under the License is distributed on an "AS IS" BASIS,
    // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    // See the License for the specific language governing permissions and
    // limitations under the License.

/******************************************************************************
 参考資料 3 LINE Notify API Document
 ******************************************************************************
 https://notify-bot.line.me/doc/ja/
 
    POST https://notify-api.line.me/api/notify
        Method  POST
        Content-Type    multipart/form-data
        Authorization   Bearer <access_token>
    パラメータ
        message String  最大 1000文字
        imageFile       png, jpeg
 ******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiClientSecure.h>
#include "esp_timer.h"
#include "esp_camera.h"

#define HTTP_WAIT 1
#ifndef PART_BOUNDARY
#define PART_BOUNDARY "123456789000000000000987654321"
#endif

#define DEBUG_HTTPS

byte LINE_Camera(const char *token, const char *message){
    int32_t i, len;
    
    /* カメラ処理部 */
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    fb = esp_camera_fb_get();
    int64_t fr_start = esp_timer_get_time();
    len = (int32_t)(fb->len);
    if(!fb || fb->format != PIXFORMAT_JPEG || len == 0){
        Serial.printf("Camera capture failed (%d)\n", ESP_FAIL);
        return ESP_FAIL;
    }

    /* HTTPS処理部 */
    WiFiClientSecure client;
    // client.setCACert(test_root_ca);
    Serial.println("\nStarting connection to server...");
    if(!client.connect("notify-api.line.me", 443)){
        Serial.println("LINE_Camera:HTTP:Connection failed");
        return ESP_FAIL;
    }
    Serial.println("HTTP Connected to LINE Notify");
    uint32_t con_len = 3 * (4 + strlen(PART_BOUNDARY)) + 2  // BOUNDARY
                    + 41                        // テキスト Content-Type
                    + 48                        // テキスト Content-Disposition
                    + strlen(message) + 2       // テキスト メッセージ
                    + 4                         // HEADER/BODY
                    + 26                        // 写真 Content-Type
                    + 70                        // 写真 Content-Disposition
                    + len                       // 写真 JPEGファイル（cam.jpg）
                    + 4                         // HEADER/BODY
                    + 2;                        // END

    client.print("POST https://notify-api.line.me/api/notify HTTP/1.0\r\n");
    client.printf("Authorization: Bearer %s\r\n", token);
//  client.print("Accept: application/json\r\n");
//  client.print("Connection: close\r\n");
    client.printf("Content-Type: multipart/form-data; boundary=%s\r\n", PART_BOUNDARY);
    client.printf("Content-Length: %u\r\n", con_len);
    client.print("\r\n");

    /* テキスト  */
    client.printf("--%s\r\n",PART_BOUNDARY);
    client.print("Content-Type: text/plain; charset=UTF-8\r\n");
    client.print("Content-Disposition: form-data; name=\"message\"\r\n");
    client.print("\r\n");
    client.printf("%s\r\n", message);
    client.print("\r\n");
    
    /* 写真  */
    client.printf("--%s\r\n",PART_BOUNDARY);
    client.print("Content-Type: image/jpeg\r\n");
    client.print("Content-Disposition: form-data; name=\"imageFile\"; filename=\"cam.jpg\"\r\n");
    client.print("\r\n");

    // for(i=0;i<len;i++) client.write(fb->buf[i]);
    i = 0;
    while(i < len ){
        int num = 1024;
        if( i + num > len) num = len - i;
        client.write((byte *)(fb->buf + i), num);
        i += num;
    }
    client.print("\r\n");
    client.printf("--%s--\r\n",PART_BOUNDARY);
    client.print("\r\n");
    client.flush();
    delay(HTTP_WAIT);

    while(client.connected()){
        String line = client.readStringUntil('\n');
        #ifdef DEBUG_HTTPS
            Serial.println(line);
        #endif
        if(line == "\r") break;
    }
    #ifdef DEBUG_HTTPS
       Serial.println();
    #endif
    while (client.available()) {
        #ifdef DEBUG_HTTPS
            char c = client.read();
            Serial.write(c);
        #endif
    }
    #ifdef DEBUG_HTTPS
       Serial.println();
    #endif
    client.stop();

    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    Serial.printf("HTTP: %u Bytes JPG: %u Bytes %u ms\n", con_len, (uint32_t)(len), (uint32_t)((fr_end - fr_start)/1000));
    client.stop();
    
    return res;
}
