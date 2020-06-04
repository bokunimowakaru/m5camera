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

void startCameraServer();
uint16_t get_face_detected_num();
esp_err_t capture_face();

/*
extern typedef struct ra_filter_t;
extern typedef struct jpg_chunking_t;
extern ra_filter_t ra_filter;
extern httpd_handle_t stream_httpd;
extern httpd_handle_t camera_httpd;
extern mtmn_config_t mtmn_config;
extern int8_t face_detection_enabled;
extern int8_t face_recognition_enabled;
extern int8_t is_enrolling;
extern int8_t pir_enabled;
extern int8_t udp_sender_enabled;
extern int8_t ftp_sender_enabled;
extern int16_t send_interval;
extern uint16_t face_detected_num;
extern face_id_list id_list;

ra_filter_t * ra_filter_init(ra_filter_t * filter, size_t sample_size);
int ra_filter_run(ra_filter_t * filter, int value);
void rgb_print(dl_matrix3du_t *image_matrix, uint32_t color, const char * str);
int rgb_printf(dl_matrix3du_t *image_matrix, uint32_t color, const char *format, ...);
void draw_face_boxes(dl_matrix3du_t *image_matrix, box_array_t *boxes, int face_id);
int run_face_recognition(dl_matrix3du_t *image_matrix, box_array_t *net_boxes);
size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len);
esp_err_t capture_handler(httpd_req_t *req);
esp_err_t stream_handler(httpd_req_t *req);
esp_err_t cmd_handler(httpd_req_t *req);
esp_err_t status_handler(httpd_req_t *req);
static esp_err_t index_handler(httpd_req_t *req);
*/
