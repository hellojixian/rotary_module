#ifndef BUZZER_H
#define BUZZER_H

#include "hal.h"

// 蜂鸣器函数声明
void buzzer_init();
void buzzer_tone(int frequency, int duration);
void play_startup_melody();

#endif // BUZZER_H
