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

#define VOLTAGE_SENSOR_PIN PC2

// 音符频率定义 (Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784

#endif // HAL_H
