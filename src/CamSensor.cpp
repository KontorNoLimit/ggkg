#include "CamSensor.h"

// (10/10/2023 kontornl) allocate memory for STATIC members
bool CamSensor::sensorInited;

CamSensor::CamSensor()
{
    this->sensorInited = false;

    // (16/04/2024 kontornl) change channel to 2 to free first 2 channel for servos
    this->cameraConfig.ledc_channel = LEDC_CHANNEL_2;
    this->cameraConfig.ledc_timer = LEDC_TIMER_2;
    this->cameraConfig.pin_d0 = Y2_GPIO_NUM;
    this->cameraConfig.pin_d1 = Y3_GPIO_NUM;
    this->cameraConfig.pin_d2 = Y4_GPIO_NUM;
    this->cameraConfig.pin_d3 = Y5_GPIO_NUM;
    this->cameraConfig.pin_d4 = Y6_GPIO_NUM;
    this->cameraConfig.pin_d5 = Y7_GPIO_NUM;
    this->cameraConfig.pin_d6 = Y8_GPIO_NUM;
    this->cameraConfig.pin_d7 = Y9_GPIO_NUM;
    this->cameraConfig.pin_xclk = XCLK_GPIO_NUM;
    this->cameraConfig.pin_pclk = PCLK_GPIO_NUM;
    this->cameraConfig.pin_vsync = VSYNC_GPIO_NUM;
    this->cameraConfig.pin_href = HREF_GPIO_NUM;
    // (16/04/2024 kontornl) change sscb to sccb according to deprecation warning
    this->cameraConfig.pin_sccb_sda = SIOD_GPIO_NUM;
    this->cameraConfig.pin_sccb_scl = SIOC_GPIO_NUM;
    this->cameraConfig.pin_pwdn = PWDN_GPIO_NUM;
    this->cameraConfig.pin_reset = RESET_GPIO_NUM;
    this->cameraConfig.xclk_freq_hz = 20000000;
//  config.xclk_freq_hz = 8000000;
    this->cameraConfig.pixel_format = PIXFORMAT_JPEG;
    /*
    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
    if(psramFound()){
    */
    this->cameraConfig.frame_size = FRAMESIZE_UXGA;
    this->cameraConfig.jpeg_quality = 10;
    this->cameraConfig.fb_count = 2;
    /*
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

#if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
#endif
    */
}

CamSensor::~CamSensor()
{
}

bool CamSensor::isSensorInited()
{
    return this->sensorInited;
}

sensor_t *CamSensor::getSensor()
{
    this->sensor = esp_camera_sensor_get();
    return this->sensor;
}

esp_err_t CamSensor::init()
{
    if(this->sensorInited) return ESP_ERR_CAMERA_BASE;
    esp_err_t err = esp_camera_init(&this->cameraConfig);
    if (err != ESP_OK) return err;
    /*
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1); // flip it back
        s->set_brightness(s, 1); // up the brightness just a bit
        s->set_saturation(s, -1); // lower the saturation
    }
    */
    // drop down frame size for higher initial frame rate
    this->sensor = esp_camera_sensor_get();
    this->sensor->set_framesize(sensor, FRAMESIZE_QVGA);
    this->sensor->set_quality(sensor, 10);
    /*
    // s->set_hmirror(s, 1);
    // s->set_vflip(s, 1);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
#endif
    */
    return ESP_OK;
}

esp_err_t CamSensor::deinit() {
    if(!this->sensorInited) return ESP_ERR_CAMERA_BASE;
    esp_err_t err = esp_camera_deinit();
    this->sensorInited = false;
    return err;
}
