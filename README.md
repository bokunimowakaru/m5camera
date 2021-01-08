# IoT M5Camera with FTP Client for Arduino IDE
## CameraWebServerFTP forked by bokunimo.net

Wi-Fi 搭載 カメラ M5Camera が顔を検知した時や人感センサ（PIR Unit）が人体などの動きを検知したとき、あるいは設定した周期ごとに、写真を撮影し、FTP サーバや、LINE、Windows PC、Raspberry Pi 等へ転送します。  

* Wi-Fi マイコン ESP32 と、JPEGカメラ OV2640 を搭載した M5Camera 用のサンプル・ソフトウェアです。  
* Espressif Systems の Arduino IDE 用 開発環境（espressif / arduino-esp32, Arduino core for the ESP32 ）に含まれる サンプル・ソフトウェアを基に、改変したものです。  
* https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/Camera/CameraWebServer  

コンパイル済みファームウェアも提供予定です。

## スマートフォンで動作確認・Raspberry Pi へ FTP 転送

スマートフォンのブラウザから M5Camera へアクセスし、動作確認が出来ます。
また、顔検知機能のON/OFFや、人感センサのON/OFF、FTP 送信のON/OFF、LINE 送信のON/OFF、送信間隔などもスマートフォンから行えます。  

![説明図1](https://github.com/bokunimowakaru/m5camera/blob/image/image/fig1.jpg)

FTP 送信時は、写真だけでなく、HTMLファイルも転送するので、パソコンのブラウザなどで写真を一覧表示することが出来ます。初期値では、16枚までの写真をHTMLファイルで表示します（int FileNumMax = 16;で変更可能）。  

![説明図2](https://github.com/bokunimowakaru/m5camera/blob/image/image/fig2.jpg)

操作画面は、日本語表示に対応したほか、検知機能のON/OFF設定と、送信ON/OFF設定、送信間隔の設定が可能です。  

![説明図3](https://github.com/bokunimowakaru/m5camera/blob/image/image/fig3.jpg)

顔を検知したときや、人感センサが反応したときに、LINE Notify へ写真を送信することも出来ます。  

![説明図4](https://github.com/bokunimowakaru/m5camera/blob/image/image/fig4.jpg)

------------------------------------------------------------------------------------
# CameraWebServerFTP の動作確認方法

本ソフトウェアを書き込んだ M5Camera (または TTGO T-Camera)を 起動すると、約10秒後に Wi-Fi アクセスポイント機能が起動します。スマートフォンまたはパソコンから、下記のSSID（パスワードなし）に接続し、インターネットブラウザでアクセスしてください。画面最下部の［写真撮影］ボタンを押すと、撮影した写真が表示されます。  

	(接続情報)
	SSID		M5CAM_XXXX (XXXXはMACアドレスの下4桁)
	PASSWORD	なし
	IPアドレス	192.168.4.1

以上だけでは、インターネットやLANへの接続が出来ません。
より実用的に使用するには、以下の手順で設定の変更を行ってください。  

# CameraWebServerFTP の設定方法

## CameraWebServerFTP のSSIDの設定変更方法

お手持ちのゲートウェイに接続するには Wi-Fi設定を行う必要があります。設定を変更しなかった場合は、前述の通り、約10秒後に本機が無線アクセスポイントとして動作します。  
CameraWebServer.ino の下記の部分を変更してください。  

	/******************************************************************************
	 Wi-Fi 設定 ※下記にゲートウェイのWi-Fi設定を入力してください
	 *****************************************************************************/
	#define WIFI_SSID   "1234ABCD"              // your wifi ssid
	#define WIFI_PASSWD "password"              // your wifi password

## CameraWebServerFTP の FTP クライアント設定方法

撮影した写真を FTP サーバへ送信することが出来ます。設定しなかった場合は、 FTP による写真転送は行いません。
FTP サーバへの写真転送機能を使用する場合は、CameraWebServer.ino の下記の部分を変更してください。  
動作確認済みのサーバについては、後述します。  

	/******************************************************************************
	 FTP 設定
	 *****************************************************************************/
	#define FTP_TO   "192.168.0.10"             // FTP 送信先のIPアドレス
	#define FTP_USER "pi"                       // FTP ユーザ名(Raspberry Pi等)
	#define FTP_PASS "your_password"            // FTP パスワード(Raspberry Pi等)
	#define FTP_DIR  "~/"                       // FTP ディレクトリ(Raspberry Pi等)
	#define Filename "cam_a_5_0000.jpg"         // FTP 保存先のファイル名

## CameraWebServerFTP の LINE Notify 設定方法

撮影した写真を LINE Notify へ送信することが出来ます。
CameraWebServer.ino の下記の部分を変更してください。  

	/******************************************************************************
	 LINE Notify 設定
	 ******************************************************************************
	 ※LINE アカウントと LINE Notify 用のトークンが必要です。
	    1. https://notify-bot.line.me/ へアクセス
	    2. 右上のアカウントメニューから「マイページ」を選択
	    3. トークン名「esp32」を入力
	    4. 送信先のトークルームを選択する（「1:1でLINE NOtifyから通知を受け取る」など）
	    5. [発行する]ボタンでトークンが発行される
	    6. [コピー]ボタンでクリップボードへコピー
	    7. 下記のLINE_TOKENに貼り付け
	 *****************************************************************************/

	#define LINE_TOKEN  "your_token"            // LINE Notify 用トークン(★要設定)
	#define MESSAGE_PIR "人感センサが反応しました。"
	#define MESSAGE_CAM "カメラが顔を検知しました。"

## CameraWebServerFTP のカメラ選択方法

使用するカメラを選択し、#defineしてください。初期状態では[選択例] M5Camera version B が選択されています。
変更する場合は、[選択例]の行に、使用するカメラを定義してください。

	/******************************************************************************
	 カメラ 選択 ※使用するカメラを選択してください。
	 *****************************************************************************/
	// CAMERA_MODEL_M5STACK_PSRAM       // M5Camera version A
	// CAMERA_MODEL_M5STACK_V2_PSRAM    // M5Camera version B
	// CAMERA_MODEL_TTGO_T_CAMERA       // TTGO T-Camera OV2640_V05用
	// CAMERA_MODEL_TTGO_T_CAMERA_V16   // TTGO T-Camera OV2640_V1.6用
	
	#define CAMERA_MODEL_M5STACK_V2_PSRAM       // [選択例] M5Camera version B

------------------------------------------------------------------------------------
# CameraWebServerFTP のコンパイル方法

Arduino IDEに、[arduino-esp32](https://github.com/espressif/arduino-esp32/releases)を組み込んで、コンパイルを行います。arduino-esp32のバージョンは 1.0.4 を使用しました。  
※ご注意：バージョン1.2.0未満には必要なライブラリが含まれていないので、動作しません。  

コンパイル時に必要なライブラリ：  
* arduino-esp32：https://github.com/espressif/arduino-esp32/releases

Arduino IDEの[ツール]メニュー⇒[ボード]から、「ESP32 Wrover Module」を選択してください。  
フラッシュメモリのスキームで、APP に 3MB以上を割り当てて下さい。  

* Arduino IDE：[ツール]⇒[ボード]⇒[ESP32 Wrover Module]
* Partition Scheme : Huge App (3MB APP No OTA/1MB SPIFFS)
* PSRAM : Enable (arduino-esp32 バージョン 1.0.4 では設定不要)

カメラ設定値を SPIFFS に保存する機能を追加しました(2020/10/18)。  
ブラウザ上の[設定保存]ボタンで設定をSPIFFSに保存します。[設定削除]ボタンで削除することもできます。  
Partition Schemeでは、上例のように、SPIFFSが利用可能なものを選択してください。  

------------------------------------------------------------------------------------
# Raspberry Pi用 サーバ

以下の3種類のサーバについて、CameraWebServerFTP との接続動作を確認済みです。実行方法は、後述します。

* Raspberry Pi 用 HTTP通信サーバ get_photo.sh  
	筆者が作成した、写真データ取得動作確認用サーバ

* Raspberry Pi 用 HTTP通信サーバ iot-cam_serv.sh  
	筆者が作成した、他の機器と連携するカメラ活用サーバ

* Raspberry Pi 用 FTPサーバ vsftpd  
	CentOSや RedHat等で使用されているFTPサーバです

## Raspberry Pi 用 HTTP通信サーバ get_photo.sh  

### get_photo

Raspberry Pi などで動作する写真データ取得動作確認用のサーバのサンプル・ソフトウェアです。
CameraWebServerFTP を書き込んだ M5Camera の電源を入れた時や、一定の周期、顔検知や顔認証センサの検知、人感センサの検知によって送信する UDP の通知を受信し、HTTP で写真を取得し、photo フォルダ内に保存します。  
M5Camera と同じWi-Fiネットワークに接続したRaspberry Piで下記のコマンドを実行するとサーバが起動します。  

	(イントールと get_photo.sh の実行方法)
	cd
	git clone https://github.com/bokunimowakaru/m5camera
	cd m5camera/tools/
	./get_photo.sh
	
	(停止方法)
	[Ctrl]+[C]

なお、悪意ある攻撃者がM5Camera と同じような通知を Raspberry Pi へ送信し、（写真では無く）ウィルスなどを Raspberry Pi 内に保存するなどの懸念があります。実験が終わったら、キーボードから[Ctrl]+[C]を入力して、停止してください。  

### UDP 通知

同じネットワーク内の M5Camera からブロードキャストで UDP 送信することにより、 M5Camera の発見と、イベントなどの通知を行います。  
CameraWebServerFTP では、ポート1024、先頭8文字にデバイス名「cam_a_5,」を付与したUDPパケットをブロードキャスト送信します。電源投入時や周期的に送信するときはデバイス名のあとに0が付与され、カメラによる顔検知や顔認証が行われたときは、検知・認証回数が付与されます。  
その後に、HTTP プロトコルを使った写真撮影用のコマンドを兼ねた URL が付与されます。  
人感センサの検知通知はデバイス名「pir_s_5,」が用いられ、人体などの動きを検知したときに1を、動きが無くなったときに0を送信します。

## Raspberry Pi 用 HTTP通信サーバ iot-cam_serv.sh

get_photo.sh と同じフォルダに格納しました。下記のコマンドを実行するとサーバが起動します。  

	(iot-cam_serv.sh の実行方法)
	cd ~/m5camera/tools/
	./iot-cam_serv.sh
	
	(停止方法)
	[Ctrl]+[C]

iot-cam_serv.sh が起動した状態で、CameraWebServerFTP を書き込んだ M5Camera の電源を入れてください（または側面のリセットボタンを押す）。このときに、カメラのIPアドレスがサーバへ送信され、それを受信した iot-cam_serv.sh は、以降、人感センサ反応などの通知を受けるたびに、写真を撮影し、Raspberry Piへ保存します。  
get_photo.sh と同様の理由から、実験が終わったら、[Ctrl]+[C]で停止してください。運用する場合はファイヤーウォールなどを適切に設定して下さい。  

## Raspberry Pi 用 FTP サーバ vsftpd

Raspberry Pi 用 FTPサーバは、以下の方法でインストールすることが出来ます。FTPサーバは、上記のソフトウェアよりもネットワーク・セキュリティの脅威にさらされやすいので、詳しくない方は、実験後、すぐに停止してください。  

	(FTPサーバ vsftpd インストールのイントールと実行)
	cd
	git clone https://github.com/bokunimowakaru/m5camera
	cd m5camera/tools/
	./ftp_setup.sh
	
	(停止)
	cd ~/m5camera/tools/
	./ftp_uninstall.sh

## Windows 用 FTP サーバ FileZilla Server

Raspberry Pi 用 FTP サーバは、FileZilla Server にて動作確認を行いました。  
送信先のディレクトリに、チルダ（ホーム）が使えないので、CameraWebServer.ino の「#define FTP_DIR "\~/"」のチルダ(\~)を削除してください。  

	(変更前)
	#define FTP_DIR  "~/"                       // FTP ディレクトリ(Raspberry Pi等)
	
	(変更後)
	#define FTP_DIR  "/"                        // FTP ディレクトリ(Windows 用)

なお、以下の設定が必要です（FileZillaに限ったことではありません）。

	(Windows システム)
	- Windows ファイヤーウォール に FileZilla Server へのアクセスを許可
	
	(FileZilla Server)
	- FTP用 アカウントの追加 (Edit -> Usrs -> (Page:General) -> Add -> ユーザ名を入力)
	- 追加したアカウントにパスワードの追加
	- Page:Shared folders に 使用するディレクトリを追加 (C\ftp)など
	- 追加したディレクトリにアクセス権の追加 (Read, Write, Delete)
	- 上記の設定で動作したら、セキュリティ対策を行う

## さくらのレンタルサーバ (2021/1/8追記)

撮影したカメラ画像を、インターネットにFTPでアップロードし、Webブラウザで表示することも出来ます。  
画像がインターネット上に公開されるので、セキュリティやモラルに十分、ご注意ください。  

さくらのレンタルサーバを契約し、以下を取得します。  

	- FTPサーバ名（例 = iot-cam.sakura.ne.jp）
	- FTPアカウント名（例 = iot-cam）
	- 初期フォルダ（例 = www）
	- サーバ用パスワード（例 = password）

上記（アカウント名＝iot-cam、初期フォルダ＝www、パスワード＝password）の場合、以下のように設定します。  

	#define FTP_TO   "iot-cam.sakura.ne.jp"  // FTP 送信先のIPアドレス(★要設定)
	#define FTP_USER "iot-cam"               // FTP ユーザ名(★要設定)
	#define FTP_PASS "password"              // FTP パスワード(★要設定)
	#define FTP_DIR  "/home/iot-cam/www"     // FTP ディレクトリ(Raspberry Pi等)

ただし、上記だと初期フォルダ www にある index.html を上書きしてしまいます。既に利用中のサーバであれば、camフォルダなどを作成し、以下のように設定して下さい。  

	#define FTP_DIR  "/home/iot-cam/www/cam" // FTP ディレクトリ(Raspberry Pi等)

ftp.ino の #define DEBUG_FTP を有効にすると、以下のようなログが表示されればFTP送信の成功です。「STOR」コマンドの後に 550 ... No such file or directory のような表示が出た場合は、サーバ上に適切なフォルダが無いか、初期フォルダ名が誤っているので修正してください。

	ftp://iot-cam:**********@iot-cam.sakura.ne.jp:21//home/iot-cam/www/cam cam_a_5_0001.jpg
	Command connected
	>220 ProFTPD 1.3.5a Server (SAKURA Internet FTP Server) [::ffff:xxx.xxx.xxx.xxx]
	USER iot-cam
	>331 Password required for iot-cam
	PASS
	>230 User iot-cam logged in
	Type i
	>200 Type set to I
	PASV
	>227 Entering Passive Mode (xxx,xxx,xxx,xxx,49,127).
	Data port: 12671
	Data connected
	STOR /home/iot-cam/www/cam/cam_a_5_0003.jpg
	>150 Opening BINARY mode data connection for /home/iot-cam/www/cam/cam_a_5_0001.jpg
	PIXFORMAT = 3
	Writing
	JPG: 10786 Bytes 282 ms
	Data disconnected
	>226 Transfer complete
	QUIT
	Command disconnected

FTPアップロードした画像が、インターネット上に公開されます。  

------------------------------------------------------------------------------------
# ご注意・外部から不正に侵入されると、室内の様子などが流出してしまいます。

運用時は、少なくともSSIDの設定やパスワードの設定が必要です。  
また、インターネット上のFTPサーバなどに転送した場合、室内の様子などが閲覧可能な状態になってしまいます。十分にご注意ください。カメラの死角となる範囲の情報が(反射や影の映り込みなどによって)流出することもあります。  
サーバなどのインターネット・セキュリティ対策も必要です。  

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

## このソースコード等には、下記の製作物が含まれます。

* Arduino core for the ESP32：  
	Espressif Systems Shanghai,China http://www.espressif.com  
	GNU LESSER GENERAL PUBLIC LICENSE Version 2.1  

* CameraWebServer (ESP32 sketches examples)：  
	Hristo Gochkov, Ivan Grokhtkov  
	GNU LESSER GENERAL PUBLIC LICENSE Version 2.1  

* app_httpd.cpp：  
	Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD  
	Apache License, Version 2.0  

## 改変部の著作権は 国野 亘 が所有します。  
ソースコードやコンテンツのフォルダなどにライセンス表示が無い著作物については、Arduino core for the ESP32 と同じライセンス形態とします。  
	Copyright 2019-2020 Wataru KUNINO  
	GNU Lesser General Public License v2.1

### 主な変更点
* Soft APモードへの自動切り替え
* UDPによる撮影完了通知機能
* FTPによる写真送信機能
* UDP、FTP、送信間隔の設定用ユーザインタフェース
* Raspberry Pi用 サーバ・ソフトウェア
* 解説・日本語訳など

--------------------------------------------------------------------------------
# 関連情報

## M5Stackの液晶ディスプレイへ表示する

Wi-Fiカメラからの画像をM5Stackへ表示することが出来るフォトフレーム用ソフトも公開中です。  

TTGO T-Camera+M5StackでWi-Fiカメラ  
* ブログ記事：https://bokunimo.net/blog/esp/420/
* レポジトリ：https://github.com/bokunimowakaru/iot-photo

by <https://bokunimo.net>
