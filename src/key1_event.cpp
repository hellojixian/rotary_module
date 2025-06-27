#include <Arduino.h>
#include "keys.h"
#include "buzzer.h"
#include "stepper_motor.h"

// 按键1按下事件处理函数
void key1_pressed() {
  // 播放按键1的专属提示音 (1900Hz)
  buzzer_tone(1900, 200);

  // 按键1功能：电机高速转动
  Serial.println(F("Key 1 pressed: Motor high speed rotation"));

  if (stepper_motor_is_running()) {
    // 如果电机正在运行，停止电机
    stepper_motor_stop();
  } else {
    // 设置高速并启动电机连续转动
    stepper_motor_set_speed(SPEED_HIGH);
    stepper_motor_set_direction(CLOCKWISE);
    stepper_motor_start();
  }
}
