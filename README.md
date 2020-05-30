# m5camera
IoT M5Camera for Arduino IDE

Wi-Fi 搭載 カメラ M5Camera が顔を検知した時や人感センサ（PIR Unit）が人体などの動きを検知したときに、写真を撮影し、FTP サーバや Windows PC、Raspberry Pi 等へ転送します。  

* Wi-Fi マイコン ESP32 と、JPEGカメラ OV2640 を搭載した M5Camera 用のサンプル・ソフトウェアです。  
* Espressif Systems の Arduino IDE 用 開発環境（espressif / arduino-esp32, Arduino core for the ESP32 ）に含まれる サンプル・ソフトウェアを基に、改変したものです。  
* https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/Camera/CameraWebServer  

コンパイル済みファームウェアも提供予定です。

## SSIDの変更方法

CameraWebServer.ino の下記の部分を変更してください。  

	/***************************************
	 *  WiFi
	 **************************************/
	#define WIFI_SSID   "1234ABCD"          // your wifi ssid
	#define WIFI_PASSWD "password"          // your wifi password


## コンパイル方法
Arduino IDEに、[arduino-esp32](https://github.com/espressif/arduino-esp32/releases)を組み込んで、コンパイルを行います。arduino-esp32のバージョンは 1.0.4 を使用しました。  
※ご注意：バージョン1.2.0未満には必要なライブラリが含まれていないので、動作しません。  

コンパイル時に必要なライブラリ：  
* arduino-esp32：https://github.com/espressif/arduino-esp32/releases

カメラ搭載ボードに合わせて#defineの設定変更が必要です。  

	コンパイル前に必要なdefine設定：  
	//#define CAMERA_MODEL_M5STACK_PSRAM	// Has PSRAM  
	//#define CAMERA_MODEL_M5STACK_V2_PSRAM	// M5Camera version B Has PSRAM  

Arduino IDEの[ツール]メニュー⇒[ボード]から、「ESP32 Wrover Module」を選択してください。  
フラッシュメモリのスキームで、APP に 3MB以上を割り当てて下さい。  
PSRAMを使用しているので、「有効」にしてください（有効／無効の選択表示がある場合のみ）。  

* Arduino IDE：[ツール]⇒[ボード]⇒[ESP32 Wrover Module]
* Partition Scheme : Huge App (3MB APP No OTA/1MB SPIFFS)
* PSRAM : Enable

## HTMLコンテンツの確認方法

	ヘッダファイルのGZIP化：
	../tools/header2gz.py
	
	内容確認：
	zcat camera_index_ov2640.html.gz
	
	GZIPの展開方法：
	gzip -d camera_index_ov2640.html.gz

## HTMLコンテンツの作成方法

	GZIP化
	gzip -k camera_index_ov2640.html
	
	GZIPのヘッダファイル化
	../tools/gz2header.py

## Raspberry Pi側の実行方法

同じWi-Fiに接続したRaspberry Piで下記のコマンドを実行するとサーバが起動します。

	git clone https://github.com/bokunimowakaru/m5camera
	cd m5camera/tools/
	./iot-cam_serv.sh

サーバが起動した状態で、M5Camera の電源を入れる、または側面のリセットボタンを押すと、カメラのIPアドレスがサーバへ送信され、以降、人感センサが反応するたびに、写真を撮影し、Raspberry Piへ保存します。

## ご注意・外部から不正に侵入されると、室内の様子などが流出してしまいます。

また、インターネット上のFTPサーバなどに転送した場合、室内の様子などが閲覧可能な状態になってしまいます。十分にご注意ください。
玄関に設置した場合であっても、カメラが死角となる範囲の情報が(反射や影の映り込みなどによって)得られたり、撮影した写真を別の写真で上書きするといったことも可能なので、運用時はSSIDやパスワードの変更が必要です。

--------------------------------------------------------------------------------
# 権利情報

## このソースコードには、下記の製作物が含まれます。

* CameraWebServer：  
	GNU LESSER GENERAL PUBLIC LICENSE Version 2.1  
* app_httpd.cpp：  
	Apache License, Version 2.0

## 改変部の権利は 国野 亘 が所有します。
	GNU Lesser General Public License v2.1

### 主な変更点
* UDPによる撮影完了通知機能
* FTPによる写真送信機能(作成中)
* UDP、FTP、送信間隔の設定用ユーザインタフェース

--------------------------------------------------------------------------------
# 関連情報

## M5Stackの液晶ディスプレイへ表示する

Wi-Fiカメラからの画像をM5Stackへ表示することが出来るフォトフレーム用ソフトも公開中です。  

TTGO T-Camera+M5StackでWi-Fiカメラ  
* ブログ記事：https://bokunimo.net/blog/esp/420/
* レポジトリ：https://github.com/bokunimowakaru/iot-photo

by <https://bokunimo.net>
