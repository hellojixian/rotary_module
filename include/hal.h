#ifndef HAL_H
#define HAL_H

#define I2C_SDA_PIN PC4
#define I2C_SCL_PIN PC5

#define OLED_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define BUZZER_PIN 9 //PB1
#define KEY0_PIN 3 //PD3
#define KEY1_PIN 4 //PD4
#define KEY2_PIN PE4
#define KEY3_PIN PE5

// configable key mapping
#define KEY_CANCEL   KEY0_PIN
#define KEY_PREV     KEY1_PIN
#define KEY_NEXT     KEY2_PIN
#define KEY_OK       KEY3_PIN

#define VOLTAGE_SENSOR_PIN PC2

#define STEP_MOTOR_INT1_PIN PE0
#define STEP_MOTOR_INT2_PIN PE1
#define STEP_MOTOR_INT3_PIN PE2
#define STEP_MOTOR_INT4_PIN PE3

// to detect if camera control cable has been plugged in, if plugged then it should be LOW, the pin should be INPUT_PULLUP
#define CAMERA_TRIGGER_SENSOR_PIN PD2
#define CAMERA_SHUTTER_TRIGGER_PIN PC0
#define CAMERA_FOCUS_TRIGGER_PIN PC1

#define CAMERA_FOCUS_TRIGGER_TIME   3000
#define CAMERA_SHUTTER_TRIGGER_TIME 4500

// 拍照模式时间配置
#define PHOTO_PRE_SHUTTER_SETTLE_TIME   1000  // 快门前停留时间（毫秒）
#define PHOTO_POST_SHUTTER_SETTLE_TIME  4500  // 快门后停留时间（毫秒）

#endif // HAL_H
