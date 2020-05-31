# IoT M5Camera for Arduino IDE

## CameraWebServer forked by bokunimo.net

Wi-Fi 搭載 カメラ M5Camera が顔を検知した時や人感センサ（PIR Unit）が人体などの動きを検知したとき、あるいは設定した周期ごとに、写真を撮影し、FTP サーバや、Windows PC、Raspberry Pi 等へ転送します。  

* Wi-Fi マイコン ESP32 と、JPEGカメラ OV2640 を搭載した M5Camera 用のサンプル・ソフトウェアです。  
* Espressif Systems の Arduino IDE 用 開発環境（espressif / arduino-esp32, Arduino core for the ESP32 ）に含まれる サンプル・ソフトウェアを基に、改変したものです。  
* https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/Camera/CameraWebServer  

コンパイル済みファームウェアも提供予定です。

------------------------------------------------------------------------------------
# CameraWebServer 設定方法

## CameraWebServer SSIDの設定変更方法

お手持ちのゲートウェイに接続するには Wi-Fi設定を行う必要があります。設定を変更しなかった場合は、約10秒後に本機が無線アクセスポイントとして動作します。  
CameraWebServer.ino の下記の部分を変更してください。  

	/******************************************************************************
	 Wi-Fi 設定 ※下記にゲートウェイのWi-Fi設定を入力してください
	 *****************************************************************************/
	#define WIFI_SSID   "1234ABCD"              // your wifi ssid
	#define WIFI_PASSWD "password"              // your wifi password

## CameraWebServer FTP クライアント設定方法

撮影した写真を FTP サーバへ送信することが出来ます。設定しなかった場合は、 FTP による写真転送は行いません。
FTP サーバへの写真転送機能を使用する場合は、CameraWebServer.ino の下記の部分を変更してください。  

	/******************************************************************************
	 FTP 設定
	 *****************************************************************************/
	#define FTP_TO   "192.168.0.10"             // FTP 送信先のIPアドレス
	#define FTP_USER "pi"                       // FTP ユーザ名(Raspberry Pi等)
	#define FTP_PASS "your_password"            // FTP パスワード(Raspberry Pi等)
	#define FTP_DIR  "~/"                       // FTP ディレクトリ(Raspberry Pi等)
	#define Filename "cam_a_5_0000.jpg"         // FTP 保存先のファイル名

## CameraWebServer カメラ選択方法

使用するカメラを選択し、#defineしてください。初期状態では[選択例] M5Camera version B が選択されています。
変更する場合は、[選択例]の行を消し、使用するカメラの行の先頭「// 」を消してください。

	/******************************************************************************
	 カメラ 選択 ※使用するカメラを選択してください。
	 *****************************************************************************/
	// #define CAMERA_MODEL_M5STACK_PSRAM       // M5Camera version A
	// #define CAMERA_MODEL_M5STACK_V2_PSRAM    // M5Camera version B
	// #define CAMERA_MODEL_TTGO_T_CAMERA       // TTGO T-Camera OV2640_V05用
	// #define CAMERA_MODEL_TTGO_T_CAMERA_V16   // TTGO T-Camera OV2640_V1.6用
	
	#define CAMERA_MODEL_M5STACK_V2_PSRAM       // [選択例] M5Camera version B

------------------------------------------------------------------------------------
# CameraWebServer コンパイル方法

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

------------------------------------------------------------------------------------
# Raspberry Pi用 サーバ

以下の3種類のサーバについて、CameraWebServer を使って、動作確認済みです。実行方法は、後述します。

* Raspberry Pi 用 HTTP通信サーバ get_photo.sh  
	筆者が作成した、写真データ取得動作確認用サーバ

* Raspberry Pi 用 HTTP通信サーバ iot-cam_serv.sh  
	筆者が作成した、他の機器と連携するカメラ活用サーバ

* Raspberry Pi 用 FTPサーバ vsftpd  
	CentOSや RedHat等で使用されているFTPサーバです

## Raspberry Pi 用 HTTP通信サーバ get_photo.sh  

同じWi-Fiに接続したRaspberry Piで下記のコマンドを実行するとサーバが起動します。  

	(イントールと get_photo.sh の実行方法)
	cd
	git clone https://github.com/bokunimowakaru/m5camera
	cd m5camera/tools/
	./get_photo.sh
	
	(停止方法)
	[Ctrl]+[C]

get_photo.sh が起動した状態で、CameraWebServer を書き込んだ M5Camera の電源を入れると、写真の取得を実行し、photoフォルダ内に写真を保存します。  
なお、悪意ある攻撃者がM5Camera と同じような通知を Raspberry Pi へ送信し、（写真では無く）ウィルスなどを Raspberry Pi 内に保存するなどの懸念があります。実験が終わったら、キーボードから[Ctrl]+[C]を入力して、停止してください。  

## Raspberry Pi 用 HTTP通信サーバ iot-cam_serv.sh

get_photo.sh と同じフォルダに格納しました。下記のコマンドを実行するとサーバが起動します。  

	(iot-cam_serv.sh の実行方法)
	cd ~/m5camera/tools/
	./iot-cam_serv.sh
	
	(停止方法)
	[Ctrl]+[C]

iot-cam_serv.sh が起動した状態で、CameraWebServer を書き込んだ M5Camera の電源を入れてください（または側面のリセットボタンを押す）。このときに、カメラのIPアドレスがサーバへ送信され、それを受信した iot-cam_serv.sh は、以降、人感センサ反応などの通知を受けるたびに、写真を撮影し、Raspberry Piへ保存します。  
get_photo.sh と同様の理由から、実験が終わったら、[Ctrl]+[C]で停止してください。運用する場合はファイヤーウォールなどを適切に設定して下さい。  

## Raspberry Pi 用 FTPサーバ vsftpd

FTPサーバは、以下の方法でインストールすることが出来ます。FTPサーバは、上記のソフトウェアよりもネットワーク・セキュリティの脅威にさらされやすいので、詳しくない方は、実験後、すぐに停止してください。  

	(FTPサーバ vsftpd インストールのイントールと実行)
	cd
	git clone https://github.com/bokunimowakaru/m5camera
	cd m5camera/tools/
	./ftp_setup.sh
	
	(停止)
	cd ~/m5camera/tools/
	./ftp_uninstall.sh

## ご注意・外部から不正に侵入されると、室内の様子などが流出してしまいます。

また、インターネット上のFTPサーバなどに転送した場合、室内の様子などが閲覧可能な状態になってしまいます。十分にご注意ください。
玄関に設置した場合であっても、カメラが死角となる範囲の情報が(反射や影の映り込みなどによって)得られたり、撮影した写真を別の写真で上書きするといったことも可能なので、運用時はSSIDやパスワードの変更が必要です。


------------------------------------------------------------------------------------
# 改造したい場合

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
