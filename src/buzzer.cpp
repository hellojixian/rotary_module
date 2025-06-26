#include <Arduino.h>
#include "buzzer.h"

// 蜂鸣器初始化函数
void buzzer_init() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

// 蜂鸣器播放音调函数
void buzzer_tone(int frequency, int duration) {
  if (frequency == 0) {
    // 静音
    digitalWrite(BUZZER_PIN, LOW);
    delay(duration);
    return;
  }

  int period = 1000000 / frequency; // 微秒
  int half_period = period / 2;
  int cycles = (long)frequency * duration / 1000;

  for (int i = 0; i < cycles; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(half_period);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(half_period);
  }
}

// 播放启动旋律函数
void play_startup_melody() {
  // 播放短促的3个上升音节
  buzzer_tone(1200, 100);
  delay(50);
  buzzer_tone(1500, 100);
  delay(50);
  buzzer_tone(2000, 100);
}
