/*******************************************************************************
FTP送信用クライアント for ESP-WROOM-02 Yahoo! ジオシティーズ対応版

                                          Copyright (c) 2016-2020 Wataru KUNINO

参考文献
  ■ 本ソースコードの作成に当たり、下記の情報を参考にしました(2016/12/14)
  |
  | FTP passive client for IDE v1.0.1 and w5100/w5200
  | Posted October 2012 by SurferTim
  | Modified 6 June 2015 by SurferTim
  |
  | http://playground.arduino.cc/Code/FTP
  
  ■ Yahoo! ジオシティーズ (Geo Cities) へのアップロード等に対応するために
  |下記への書き込み情報を参考にしました。
  | esp8266/Arduino Ussues
  | Add FTP Client library  #1183
  |
  | https://github.com/esp8266/Arduino/issues/1183
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"

#define TIMEOUT 20000                       // タイムアウト 20秒
#define FTP_WAIT 1
#define BUFFER_SIZE 128

byte FTP_Sender(const char *filename, int photo_num, boolean overflow){
    char ftpBuf[BUFFER_SIZE];
    WiFiClient client;
    WiFiClient dclient;
    int32_t i, len;
    
    Serial.println("ftp://" + String(FTP_USER) + ":"\
        + String(FTP_PASS) + "@" + String(FTP_TO) + ":21/"\
        + String(FTP_DIR) + " " + String(filename)\
    );

    int64_t fr_start = esp_timer_get_time();

    if (client.connect(FTP_TO,21)) {
        Serial.println("Command connected");
    }
    if(eRcv(client,ftpBuf)) return 21;

    sprintf(ftpBuf,"USER %s\r\n",FTP_USER);
    client.print(ftpBuf);
    delay(FTP_WAIT);
    Serial.print(ftpBuf);
    if(eRcv(client,ftpBuf)) return 22;

    sprintf(ftpBuf,"PASS %s\r\n",FTP_PASS);
    client.print(ftpBuf);
    delay(FTP_WAIT);
    Serial.println("PASS");
    if(eRcv(client,ftpBuf)) return 23;
    
    client.print("Type I\r\n");
    delay(FTP_WAIT);
    Serial.println("Type i");
    if(eRcv(client,ftpBuf)) return 25;

    /* PASVモード設定 */
    client.print("PASV\r\n");
    delay(FTP_WAIT);
    Serial.println("PASV");
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
    Serial.print("Data port: ");
    hiPort = hiPort | loPort;
    Serial.println(hiPort);
    if (dclient.connect(FTP_TO,hiPort)) {
        Serial.println("Data connected");
    }else{
        Serial.println("Data connection failed");
        client.stop();
//      file.close();
        return 31;
    }
    
    /* カメラ用ファイル */
    sprintf(ftpBuf,"STOR %s/%s\r\n",FTP_DIR,filename);
    client.print(ftpBuf);
    delay(FTP_WAIT);
    Serial.print(ftpBuf);
    if(eRcv(client,ftpBuf)){
        dclient.stop();
        client.stop();
//      file.close();
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
    Serial.printf("PIXFORMAT = %d\n", fb->format);
    if(fb->format == PIXFORMAT_JPEG){
        len = (int)(fb->len);
        // res = httpd_resp_send(req, (const char *)fb->buf, fb->len);

        Serial.println("Writing");
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
        Serial.printf("Camera capture failed: Unspupported format\n");
        dclient.stop();
        client.stop();
        return 11;
    }
    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    Serial.printf("JPG: %u Bytes %u ms\n", (uint32_t)(len), (uint32_t)((fr_end - fr_start)/1000));
    dclient.stop();
    Serial.println("Data disconnected");
    if(eRcv(client,ftpBuf)) return 33;
    
/*
    client.print("PASV\r\n");
    delay(FTP_WAIT);
    Serial.println("PASV");
    delay(100);
    if(eRcv(client,ftpBuf)) return 26;

    tStr = strtok(ftpBuf,"(,");
    if(tStr == NULL) return 27;
    for (i = 0; i < 6; i++) {
        tStr = strtok(NULL,"(,");
        array_pasv[i] = atoi(tStr);
        if(tStr == NULL){
            Serial.println("Bad PASV Answer");
            return 41;
        }
    }
    hiPort = array_pasv[4] << 8;
    loPort = array_pasv[5] & 255;
    Serial.print("Data port: ");
    hiPort = hiPort | loPort;
    Serial.println(hiPort);
    if (dclient.connect(FTP_TO,hiPort)) {
        Serial.println("Data connected");
    }else{
        Serial.println("Data connection failed");
        client.stop();
//      file.close();
        return 42;
    }
    
    sprintf(ftpBuf,"STOR %s/index.html\r\n",FTP_DIR);
    client.print(ftpBuf);
    delay(FTP_WAIT);
    Serial.print(ftpBuf);
    if(eRcv(client,ftpBuf)){
        dclient.stop();
        client.stop();
//      file.close();
        return 43;
    }
	len = sizeof(Filename);
	if(len > 17){
        dclient.stop();
        client.stop();
        return 44;
	}
	char s[17] = Filename;
    if(dclient.connected()){
    	dclient.println("<html><meta http-equiv=\"refresh\" content=\"20;\"><h1>ESP32 Camera</h1>");
    	for(i = photo_num; i >= 1; i--){
			int div = 1;
			for(int j=0; j<4; j++){
				s[len - 6 - j] = '0' + ((i / div) % 10);
				div *= 10;
			}
			Serial.print("Filename = ");
			Serial.println(s);
			dclient.printf("<img src = \"%s\">\n",s);
		}
		if(overflow){
			for(int j=0; j<4; j++) s[len - 6 - j] = '0';
			Serial.print("Filename = ");
			Serial.println(s);
			dclient.printf("<img src = \"%s\">\n",s);
			for(int i=FileNumMax; i >= photo_num + 2; i--){
				int div = 1;
				for(int j=0; j<4; j++){
					s[len - 6 - j] = '0' + ((i / div) % 10);
					div *= 10;
				}
				Serial.print("Filename = ");
				Serial.println(s);
				dclient.printf("<img src = \"%s\">\n",s);
			}
		}
		dclient.println("</html>");
        dclient.flush();
    }
    dclient.stop();
    Serial.println("Data disconnected");
    if(eRcv(client,ftpBuf)) return 45;
*/
    client.print("QUIT\r\n");
    delay(FTP_WAIT);
    Serial.println("QUIT");
    if(eRcv(client,ftpBuf)) return 91;
    client.stop();
    Serial.println("Command disconnected");
//  file.close();
//  Serial.println("SPIFFS closed");
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
            Serial.write('>');
            Serial.println(ftpBuf);
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
