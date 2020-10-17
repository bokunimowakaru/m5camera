#include <SPIFFS.h>
extern char CONFIGFILE[];
extern int wake;
extern int8_t face_detection_enabled;
extern int8_t face_recognition_enabled;
extern int8_t pir_enabled;
extern int8_t udp_sender_enabled;
extern int8_t ftp_sender_enabled;
extern int8_t line_sender_enabled;
extern int16_t send_interval;

esp_err_t cameraMyConfig(){
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
        return err;
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

    if(SPIFFS.exists(CONFIGFILE)){
        File file = SPIFFS.open(CONFIGFILE,"r");
        if(file){
            Serial.printf("Loading %s(%d bytes)\n",CONFIGFILE,sizeof(sensor_t));
            file.read((byte *)s, sizeof(sensor_t));
            file.close();
            printCamStatus(s);	// app_httpd.h
        }
    }
    return ESP_OK;
}
