/******************************************************************************
 権利情報：このソースコード等には、下記の製作物が含まれます。
    - wakeup.ino 
        MIT License (ファイル=wakeup_LICENSE)
        https://github.com/m5stack/TimerCam-arduino
    - 改変部の著作権は Wataru KUNINO (bokunimo.net) が所有します。
 *****************************************************************************/

// #define CAMERA_LED_GPIO 2
#define BAT_OUTPUT_HOLD_PIN 33
#define BAT_ADC_PIN 38
#define Ext_PIN_1 4
#define Ext_PIN_2 13

#ifdef CAMERA_LED_GPIO
void led_breathe_test() {
  for (int16_t i = 0; i < 1024; i++) {
    led_brightness(i);
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  for (int16_t i = 1023; i >= 0; i--) {
    led_brightness(i);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
#endif

void setupTimerCAM() {
  // Serial.begin(115200);

  // will hold bat output
  bat_init();

  #ifdef CAMERA_LED_GPIO
    led_init(CAMERA_LED_GPIO);
  #endif
  bmm8563_init();

  // 5 sec later will wake up
  // bmm8563_setTimerIRQ(5);
  
  // rtc_date_t date;
  // date.year = 2020;
  // date.month = 9;
  // date.day = 20;
  // date.hour = 15;
  // date.minute = 59;
  // date.second = 06;
  // bmm8563_setTime(&date);
  #ifdef CAMERA_LED_GPIO
    led_breathe_test();
  #endif
}

void sleepTimerCAM(uint32_t us) {
  rtc_date_t date;
  bmm8563_getTime(&date);
  Serial.printf("Time: %d/%d/%d %02d:%02d:%-2d\r\n", date.year, date.month, date.day, date.hour, date.minute, date.second);
  Serial.printf("volt: %d mv\r\n", bat_get_voltage());

  bmm8563_setTimerIRQ((int)(us / 1000000));
  // disable bat output, will wake up after 5 sec, Sleep current is 1~2μA
  bat_disable_output();

  // if usb not connect, will not in here;
  esp_deep_sleep(us);
  esp_deep_sleep_start();
}
