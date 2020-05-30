#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>						// UDP通信を行うライブラリ

/******************************************************************************
 カメラ 選択 ※使用するカメラを選択してください。
 *****************************************************************************/
// #define CAMERA_MODEL_M5STACK_PSRAM		// M5Camera version A
// #define CAMERA_MODEL_M5STACK_V2_PSRAM 	// M5Camera version B
// #define CAMERA_MODEL_TTGO_T_CAMERA		// TTGO T-Camera OV2640_V05用
// #define CAMERA_MODEL_TTGO_T_CAMERA_V16	// TTGO T-Camera OV2640_V1.6用

#define CAMERA_MODEL_M5STACK_V2_PSRAM		// [選択例] M5Camera version B
#include "camera_pins.h"					// 選択したカメラの設定値の組込部

/******************************************************************************
 Wi-Fi 設定 ※下記にゲートウェイのWi-Fi設定を入力してください
 *****************************************************************************/
#define WIFI_SSID	"1234ABCD"				// your wifi ssid
#define WIFI_PASSWD "password"				// your wifi password

/******************************************************************************
 FTP 設定
 ******************************************************************************
 ※FTPサーバが必要です。
 　ラズベリーパイへ、FTPサーバをセットアップするには下記を実行してください。
 　~/m5camera/tools/ftp_setup.sh
 ※FTPでは、ユーザ名やパスワード、データーが平文で転送されます。
 　インターネット上で扱う場合は、セキュリティに対する配慮が必要です。
 *****************************************************************************/
#define FTP_TO   "192.168.0.10"             // FTP 送信先のIPアドレス
#define FTP_USER "pi"                       // FTP ユーザ名(Raspberry Pi)
#define FTP_PASS "password"                 // FTP パスワード(Raspberry Pi)
#define FTP_DIR  "~/"                       // FTP ディレクトリ(Raspberry Pi)
#define Filename "cam_a_5_0000.jpg"         // FTP 保存先のファイル名
#define FileNumMax 16                       // FTP 保存先の最大ファイル数
int     FileNum = 1;						// 現在のファイル番号
boolean FileNumOVF = false;					// オーバーフロー用フラグ

/******************************************************************************
 UDP 設定
 *****************************************************************************/
#define PORT 1024							// UDP送信先ポート番号
#define DEVICE_CAM  "cam_a_5,"				// デバイス名(カメラ)
#define DEVICE_PIR  "pir_s_5,"				// デバイス名(人感センサ)

/******************************************************************************
 コンパイル時に、[ESP32 Wrover Module]と[Huge App]を選択してください
 ******************************************************************************
  * Arduino IDE：[ツール]⇒[ボード]⇒[ESP32 Wrover Module]
  * Partition Scheme : [Huge App (3MB APP No OTA/1MB SPIFFS)]
  * PSRAM : Enable
 *****************************************************************************/

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//			  Ensure ESP32 Wrover Module or other board with PSRAM is selected
//			  Partial images will be transmitted if image exceeds buffer size
//

void startCameraServer();
uint16_t get_face_detected_num();

int8_t face_detection_enabled = 0;			// 顔検知 無効=0,有効=1
int8_t face_recognition_enabled = 0;		// 顔認証 無効=0,有効=1
int8_t pir_enabled = 1;						// PIR 人感センサ無効=0,有効=1
int8_t udp_sender_enabled = 1;				// UDP送信無効=0,有効=1
int8_t ftp_sender_enabled = 0;				// FTP送信無効=0,有効=1
int16_t send_interval = 30;					// 送信間隔（秒）
IPAddress IP_LOCAL;							// 本機のIPアドレス
IPAddress IP_BROAD;							// ブロードキャストIPアドレス
int pir=0;									// 人感センサ値
int send_c = 0;								// タイマー

void sendUdp(String dev, String S){
	WiFiUDP udp;							// UDP通信用のインスタンスを定義
	udp.beginPacket(IP_BROAD, PORT);		// UDP送信先を設定
	udp.println(dev + S);
	udp.endPacket();						// UDP送信の終了(実際に送信する)
	Serial.println("udp://" + IP_BROAD.toString() + ":" + PORT + " " + dev + S);
	delay(200); 							// 送信待ち時間
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

void setup() {
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	Serial.println();
	Serial.println("M5Camera started");
	
	// 人感センサ
	if(PIR_GPIO_NUM > 0){
		pinMode(PIR_GPIO_NUM, INPUT);
		Serial.printf("PIR = %d, ", digitalRead(PIR_GPIO_NUM));
		delay(100);
		Serial.printf("%d, ", digitalRead(PIR_GPIO_NUM));
		delay(2000 * digitalRead(PIR_GPIO_NUM));
		Serial.printf("%d\n", digitalRead(PIR_GPIO_NUM));
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
	
	// if PSRAM IC present, init with UXGA resolution and higher JPEG quality
	//						for larger pre-allocated frame buffer.
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
	WiFi.begin(WIFI_SSID, WIFI_PASSWD);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	IP_LOCAL = WiFi.localIP();
	Serial.println(IP_LOCAL);
	Serial.println("WiFi connected");

	startCameraServer();
	Serial.print("Camera Ready! Use 'http://");
	Serial.print(IP_LOCAL);
	Serial.println("' to connect");

	IP_BROAD = IP_LOCAL;
	IP_BROAD[3] = 255;
	sendUdp_Ident();
}

void loop() {
	uint16_t face = get_face_detected_num();
	Serial.printf(
		"%04d/%04d, pir:%d,(%d), face:%d,%d,(%d), udp:%d, ftp:%d\n",
		send_c,send_interval,
		pir_enabled,(pir),
		face_detection_enabled,face_recognition_enabled,(face),
		udp_sender_enabled,ftp_sender_enabled
	);
	
	/* トリガ①：カメラによる顔検出 */
	if((face_detection_enabled || face_recognition_enabled) && face){
		Serial.println("Triggered by Face Detector");
		sendUdp_Fd(face);
		send_c = 0;
	}
	
	/* トリガ②：人感センサ（PIR Unit）による動体検出 */
	if(pir_enabled && PIR_GPIO_NUM > 0 && pir != digitalRead(PIR_GPIO_NUM) ){
		pir = !pir;
		if(pir){
			Serial.println("Triggered by PIR");
			send_c = 0;
		}
		sendUdp_Pir(pir);
		// M5 STACK PIR UNIT の信号保持時間は2秒なので1秒毎の確認でOK
	}
	
	/* トリガ③：インターバル・タイマ設定時間の経過 */
	if(send_interval && send_c >= send_interval){
		Serial.println("Triggered by Interval Timer");
		if(udp_sender_enabled) sendUdp_Ident();
		send_c = 0;
	}
	
	/* FTP送信（トリガ発生時） */
	if(ftp_sender_enabled && !send_c ){
		int len = sizeof(Filename);
		if(len <= 17){
			char s[17] = Filename;
			int div = 1;
			for(int i=0; i<4; i++){
				s[len - 6 - i] = '0' + ((FileNum / div) % 10);
				div *= 10;
			}
			FTP_Sender(s, FileNum, FileNumOVF);
			FileNum++;
			if(FileNum > FileNumMax){
				FileNum = 0;		// FileNumMaxよりも1だけ多く(書き換え用)
				FileNumOVF = true;	// 1周した
			}
		} else Serial.printf("ERROR: length of Filename %d\n",len);
	}
	delay(1000 - (send_c ? 0 : 200) - (face ? 200 : 0));
	if(send_interval) send_c++; else send_c = 1;
}
