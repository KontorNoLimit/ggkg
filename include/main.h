#ifndef __MAIN_H__
#define __MAIN_H__

// #define CORE_DEBUG_LEVEL ARDUHAL_LOG_LEVEL_DEBUG

#include <Arduino.h>
#include "esp_http_server.h"
#include "nvs_flash.h"
#include "config.h"
#include "CamSensor.h"
#include "Gimbal.h"

#define WLAN_CONN_TIME_MAX      10e3

#define LED_FLASH 4
#define LED_BUILTIN 33

extern bool camera_is_inited, isStreaming;
extern uint8_t flash_br;
extern HardwareSerial uart0;
extern Gimbal gimbal;
extern CamSensor camSensor;
extern String httpd_auth_b64;
extern char hostmsg[];
extern char *hostamsg;

// void net_asm();
void startCameraServer();
esp_err_t stopCameraServer();

#endif
