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

rtc_date_t rtc_cc_date;

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

unsigned int rtc_date_t2ui(rtc_date_t *date){
    unsigned int ret = (date->year - 2021);
    ret = ret * 12 + (date->month - 1);
    ret = ret * 31 + (date->day - 1);
    ret = ret * 24 + (date->hour);
    ret = ret * 60 + (date->minute);
    ret = ret * 60 + (date->second);
    return ret;
}

boolean rtc_wakeup_reason(){
  rtc_date_t date;
  bmm8563_getTime(&date);
  if(rtc_date_t2ui(&date) <= rtc_date_t2ui(&rtc_cc_date) + 10) return 0;
  return 3;
}

boolean setupTimerCAM(boolean set_rtc) {
  boolean ret = false;
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
  
  rtc_date_t date;
  bmm8563_getTime(&date);
  rtc_cc_date.hour = atoi(__TIME__);
  rtc_cc_date.minute = atoi(__TIME__ + 3);
  rtc_cc_date.second = atoi(__TIME__ + 6);
  rtc_cc_date.day = atoi(__DATE__ + 4);
  rtc_cc_date.year = atoi(__DATE__ + 7);
  const char m_s[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  int m = 12;
  for(; m>1; m--){
    if( strncmp(m_s[m-1], __DATE__, 3) == 0) break;
  }
  rtc_cc_date.month = m;
  int t = bmm8563_getTimerTime();

  Serial.printf("CC  Time: %d/%d/%d %02d:%02d:%02d\r\n", rtc_cc_date.year, rtc_cc_date.month, rtc_cc_date.day, rtc_cc_date.hour, rtc_cc_date.minute, rtc_cc_date.second);
  Serial.printf("RTC Time: %d/%d/%d %02d:%02d:%02d\r\n", date.year, date.month, date.day, date.hour, date.minute, date.second);
  Serial.printf("TimerTime: %d\n",t);

  if(set_rtc || rtc_date_t2ui(&date) <= rtc_date_t2ui(&rtc_cc_date)){
    Serial.printf("SET_RTC %s %s\n",__DATE__,__TIME__);
    bmm8563_setTime(&rtc_cc_date);
    ret = true;
  }
  
  #ifdef CAMERA_LED_GPIO
    led_breathe_test();
  #endif
  return ret;
}

void sleepTimerCAM(uint32_t us) {
  rtc_date_t date;
  bmm8563_getTime(&date);
  Serial.printf("RTC Time: %d/%d/%d %02d:%02d:%02d\r\n", date.year, date.month, date.day, date.hour, date.minute, date.second);
  Serial.printf("volt: %d mv\r\n", bat_get_voltage());

  bmm8563_setTimerIRQ((int)(us / 1000000));
  // disable bat output, will wake up after 5 sec, Sleep current is 1~2μA
  bat_disable_output();

  // if usb not connect, will not in here;
  esp_deep_sleep(us);
  esp_deep_sleep_start();
}
