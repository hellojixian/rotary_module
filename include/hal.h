#ifndef HAL_H
#define HAL_H

#define I2C_SDA_PIN PC4
#define I2C_SCL_PIN PC5

#define OLED_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define BUZZER_PIN 9 //PB1
#define KEY0_PIN PD3
#define KEY1_PIN PD4
#define KEY2_PIN PE4
#define KEY3_PIN PE5

// 蜂鸣器函数声明
void buzzer_init();
void buzzer_tone(int frequency, int duration);
void play_startup_melody();

#endif // HAL_H
