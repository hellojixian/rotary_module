#include <Arduino.h>
#include "keys.h"
#include "buzzer.h"
#include "stepper_motor.h"

// 按键0按下事件处理函数
void key0_pressed() {
  // 播放按键0的专属提示音 (1800Hz)
  buzzer_tone(1800, 200);

  // 按键0功能：电机低速转动
  Serial.println(F("Key 0 pressed: Motor low speed rotation"));

  if (stepper_motor_is_running()) {
    // 如果电机正在运行，停止电机
    stepper_motor_stop();
  } else {
    // 设置低速并启动电机连续转动
    stepper_motor_set_speed(SPEED_LOW);
    stepper_motor_set_direction(CLOCKWISE);
    stepper_motor_start();
  }
}
