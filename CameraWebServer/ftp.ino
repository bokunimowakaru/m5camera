/*******************************************************************************
FTP送信用クライアント

                                          Copyright (c) 2016-2020 Wataru KUNINO
********************************************************************************
参考文献
  ■ 本ソースコードの作成に当たり、下記の情報を参考にしました(2016/12/14)
  |
  | FTP passive client for IDE v1.0.1 and w5100/w5200
  | Posted October 2012 by SurferTim
  | Modified 6 June 2015 by SurferTim
  |
  | http://playground.arduino.cc/Code/FTP
  
  ■ Yahoo! ジオシティーズ (Geo Cities) へのアップロード対応
  |下記への書き込み情報を参考にしました。
  | esp8266/Arduino Ussues
  | Add FTP Client library  #1183
  |
  | https://github.com/esp8266/Arduino/issues/1183
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include "esp_timer.h"
#include "esp_camera.h"

#define FTP_WAIT 1
#define BUFFER_SIZE 128

//#define DEBUG_FTP

byte FTP_Camera(const char *filename){
    char ftpBuf[BUFFER_SIZE];
    WiFiClient client;
    WiFiClient dclient;
    int32_t i, len;
    
    char pass[17];
    memset(pass,'*',16);
    len = strlen(FTP_PASS);
    if(len < 17) pass[len]=0; else pass[16]=0;
    Serial.println("ftp://" + String(FTP_USER) + ":"\
        + String(pass) + "@" + String(FTP_TO) + ":21/"\
        + String(FTP_DIR) + " " + String(filename)\
    );

    int64_t fr_start = esp_timer_get_time();

    if (client.connect(FTP_TO,21)) {
        #ifdef DEBUG_FTP
            Serial.println("Command connected");
        #endif
    }
    if(eRcv(client,ftpBuf)) return 21;

    sprintf(ftpBuf,"USER %s\r\n",FTP_USER);
    client.print(ftpBuf);
    delay(FTP_WAIT);
    #ifdef DEBUG_FTP
        Serial.print(ftpBuf);
    #endif
    if(eRcv(client,ftpBuf)) return 22;

    sprintf(ftpBuf,"PASS %s\r\n",FTP_PASS);
    client.print(ftpBuf);
    delay(FTP_WAIT);
    #ifdef DEBUG_FTP
        Serial.println("PASS");
    #endif
    if(eRcv(client,ftpBuf)) return 23;
    
    client.print("Type I\r\n");
    delay(FTP_WAIT);
    #ifdef DEBUG_FTP
        Serial.println("Type i");
    #endif
    if(eRcv(client,ftpBuf)) return 25;

    /* PASVモード設定 */
    client.print("PASV\r\n");
    delay(FTP_WAIT);
    #ifdef DEBUG_FTP
        Serial.println("PASV");
    #endif
    delay(100);
    if(eRcv(client,ftpBuf)) return 26;

    char *tStr = strtok(ftpBuf,"(,");
    if(tStr == NULL) return 27;
    int array_pasv[6];
    for (i = 0; i < 6; i++) {
        tStr = strtok(NULL,"(,");
        array_pasv[i] = atoi(tStr);
        if(tStr == NULL){
            Serial.println("Bad PASV Answer");
            return 28;
        }
    }
    
    unsigned int hiPort,loPort;
    hiPort = array_pasv[4] << 8;
    loPort = array_pasv[5] & 255;
    #ifdef DEBUG_FTP
        Serial.print("Data port: ");
    #endif
    hiPort = hiPort | loPort;
    #ifdef DEBUG_FTP
    Serial.println(hiPort);
    #endif
    if (dclient.connect(FTP_TO,hiPort)) {
        #ifdef DEBUG_FTP
            Serial.println("Data connected");
        #endif
    }else{
        Serial.println("Data connection failed");
        client.stop();
        return 31;
    }
    
    /* カメラ用ファイル */
    //sprintf(ftpBuf,"STOR %s/%s\r\n",FTP_DIR,filename);
    sprintf(ftpBuf,"STOR %s",FTP_DIR);
    if(ftpBuf[strlen(ftpBuf)-1] != '/') strcat(ftpBuf,"/");
    strcat(ftpBuf,filename);
    strcat(ftpBuf,"\r\n");
    client.print(ftpBuf);
    delay(FTP_WAIT);
    #ifdef DEBUG_FTP
        Serial.print(ftpBuf);
    #endif
    if(eRcv(client,ftpBuf)){
        Serial.println("STOR failed");
        dclient.stop();
        client.stop();
        return 32;
    }
    
    /* カメラ処理部 */
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.printf("Camera capture failed (%d)\n", ESP_FAIL);
        dclient.stop();
        client.stop();
        return 10;
    }
    #ifdef DEBUG_FTP
        Serial.printf("PIXFORMAT = %d\n", fb->format);
    #endif
    if(fb->format == PIXFORMAT_JPEG){
        len = (int)(fb->len);
        // res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        #ifdef DEBUG_FTP
            Serial.println("Writing");
        #endif
        if(dclient.connected() && len > 0){
            // for(i=0;i<len;i++) dclient.write(fb->buf[i]);
            i = 0;
            while(i < len ){
                int num = 1024;
                if( i + num > len) num = len - i;
                dclient.write((byte *)(fb->buf + i), num);
                i += num;
            }
            dclient.flush();
        }
    } else {
        Serial.printf("Camera capture failed: Unspupported format (%d)\n", fb->format);
        dclient.stop();
        client.stop();
        return 11;
    }
    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    Serial.printf("JPG: %u Bytes %u ms\n", (uint32_t)(len), (uint32_t)((fr_end - fr_start)/1000));
    dclient.stop();
    #ifdef DEBUG_FTP
        Serial.println("Data disconnected");
    #endif
    if(eRcv(client,ftpBuf)) return 33;
    
    client.print("QUIT\r\n");
    delay(FTP_WAIT);
    #ifdef DEBUG_FTP
        Serial.println("QUIT");
    #endif
    if(eRcv(client,ftpBuf)) return 91;
    client.stop();
    #ifdef DEBUG_FTP
        Serial.println("Command disconnected");
    #endif
    return 0;
}

byte FTP_Html(int photo_num, boolean overflow){
    char filename[] = "index.html";
    char ftpBuf[BUFFER_SIZE];
    WiFiClient client;
    WiFiClient dclient;
    int32_t i, len;
    
    char pass[17];
    memset(pass,'*',16);
    len = strlen(FTP_PASS);
    if(len < 17) pass[len]=0; else pass[16]=0;
    Serial.println("ftp://" + String(FTP_USER) + ":"\
        + String(pass) + "@" + String(FTP_TO) + ":21/"\
        + String(FTP_DIR) + " " + String(filename)\
    );

    int64_t fr_start = esp_timer_get_time();

    if (client.connect(FTP_TO,21)) {
        #ifdef DEBUG_FTP
            Serial.println("Command connected");
        #endif
    }
    if(eRcv(client,ftpBuf)) return 21;

    sprintf(ftpBuf,"USER %s\r\n",FTP_USER);
    client.print(ftpBuf);
    delay(FTP_WAIT);
    #ifdef DEBUG_FTP
        Serial.print(ftpBuf);
    #endif
    if(eRcv(client,ftpBuf)) return 22;

    sprintf(ftpBuf,"PASS %s\r\n",FTP_PASS);
    client.print(ftpBuf);
    delay(FTP_WAIT);
    #ifdef DEBUG_FTP
        Serial.println("PASS");
    #endif
    if(eRcv(client,ftpBuf)) return 23;
    
    client.print("Type A\r\n");     ////////////////// I -> A
    delay(FTP_WAIT);
    #ifdef DEBUG_FTP
        Serial.println("Type A");   ////////////////// I -> A
    #endif
    if(eRcv(client,ftpBuf)) return 25;

    /* PASVモード設定 */
    client.print("PASV\r\n");
    delay(FTP_WAIT);
    #ifdef DEBUG_FTP
        Serial.println("PASV");
    #endif
    delay(100);
    if(eRcv(client,ftpBuf)) return 26;

    char *tStr = strtok(ftpBuf,"(,");
    if(tStr == NULL) return 27;
    int array_pasv[6];
    for (i = 0; i < 6; i++) {
        tStr = strtok(NULL,"(,");
        array_pasv[i] = atoi(tStr);
        if(tStr == NULL){
            Serial.println("Bad PASV Answer");
            return 28;
        }
    }
    
    unsigned int hiPort,loPort;
    hiPort = array_pasv[4] << 8;
    loPort = array_pasv[5] & 255;
    #ifdef DEBUG_FTP
        Serial.print("Data port: ");
    #endif
    hiPort = hiPort | loPort;
    #ifdef DEBUG_FTP
    Serial.println(hiPort);
    #endif
    if (dclient.connect(FTP_TO,hiPort)) {
        #ifdef DEBUG_FTP
            Serial.println("Data connected");
        #endif
    }else{
        Serial.println("Data connection failed");
        client.stop();
        return 31;
    }
    
    /* HTMLファイル */
    sprintf(ftpBuf,"STOR %s",FTP_DIR);
    if(ftpBuf[strlen(ftpBuf)-1] != '/') strcat(ftpBuf,"/");
    strcat(ftpBuf,"index.html\r\n");
    client.print(ftpBuf);
    delay(FTP_WAIT);
    #ifdef DEBUG_FTP
        Serial.print(ftpBuf);
    #endif
    if(eRcv(client,ftpBuf)){
        Serial.println("STOR failed");
        dclient.stop();
        client.stop();
        return 43;
    }
    int s_len = strlen(Filename); // 写真のファイル名（index.htmlではない）
    if(s_len > 16 || s_len < 9){
        Serial.printf("len of Filename = %d\n",s_len);
        dclient.stop();
        client.stop();
        return 44;
    }
    char s[17] = Filename;
    if(dclient.connected()){
        dclient.println("<html><meta http-equiv=\"refresh\" content=\"20;\"><h1>ESP32 Camera</h1>");
        len = 72 - 4 + 1;
        for(i = photo_num; i >= 1; i--){
            int div = 1;
            for(int j=0; j<4; j++){
                s[s_len - 5 - j] = '0' + ((i / div) % 10);
                div *= 10;
            }
            // Serial.print("Filename = "); Serial.println(s);
            dclient.printf("<img src = \"%s\">\n",s);
            len += strlen(s) + 20 - 3 - 2;
        }
        if(overflow){
            for(int j=0; j<4; j++) s[s_len - 5 - j] = '0';
            if(photo_num != 16){
                // Serial.print("Filename = "); Serial.println(s);
                dclient.printf("<img src = \"%s\">\n",s);
                len += strlen(s) + 20 - 3 - 2;
            }
            for(int i=FileNumMax; i >= photo_num + 2; i--){
                int div = 1;
                for(int j=0; j<4; j++){
                    s[s_len - 5 - j] = '0' + ((i / div) % 10);
                    div *= 10;
                }
                // Serial.print("Filename = "); Serial.println(s);
                dclient.printf("<img src = \"%s\">\n",s);
                len += strlen(s) + 20 - 3 - 2;
            }
        }
        dclient.println("</html>");
        len += 7;
        dclient.flush();
    }
    int64_t fr_end = esp_timer_get_time();
    Serial.printf("HTML: %u Bytes %u ms\n", (uint32_t)(len), (uint32_t)((fr_end - fr_start)/1000));
    dclient.stop();
    #ifdef DEBUG_FTP
        Serial.println("Data disconnected");
    #endif
    if(eRcv(client,ftpBuf)) return 33;
    
    client.print("QUIT\r\n");
    delay(FTP_WAIT);
    #ifdef DEBUG_FTP
        Serial.println("QUIT");
    #endif
    if(eRcv(client,ftpBuf)) return 91;
    client.stop();
    #ifdef DEBUG_FTP
        Serial.println("Command disconnected");
    #endif
    return 0;
}

byte eRcv(WiFiClient &client,char *ftpBuf){
    byte thisByte,i=0,len=0;

    while(!client.available()){
        delay(FTP_WAIT);
        if(!client.connected()){
            Serial.println("FTP:eRcv:disC");
            return 11;
        }
        i++;
        if(i>1000){ // 200ms以上必要
            Serial.println("FTP:eRcv:noRes");
            return 12;
        }
    }
    while(client.connected()){
        if(!client.available()){
            delay(FTP_WAIT);
            if(!client.available()) break;
        }
        thisByte = client.read();
        if(thisByte==(byte)'\r');
        else if(thisByte==(byte)'\n'){
            #ifdef DEBUG_FTP
                Serial.write('>');
                Serial.println(ftpBuf);
            #endif
            if(ftpBuf[0] >= '4'){
                client.print("QUIT\r\n");
                delay(FTP_WAIT);
                Serial.println("QUIT");
                return 1;
            }
            if(len>3 && ftpBuf[3] == ' '){
                return 0;
            }
            len = 0;
        }else if(len < BUFFER_SIZE - 1 ){
            ftpBuf[len] = thisByte;
            len++;      
            ftpBuf[len] = 0;
        }
    }
    return 0;
}

void efail(WiFiClient &client){
    byte thisByte = 0;
    client.print("QUIT\r\n");
    delay(FTP_WAIT);
    while(!client.available()){
        if(!client.connected()) return;
        delay(1);
    }
    while(client.available()){  
        thisByte = client.read();
        Serial.write(thisByte);
    }
    client.stop();
    Serial.println("Command disconnected");
}
