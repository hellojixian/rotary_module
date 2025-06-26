#include <Arduino.h>
#include "keys.h"
#include "buzzer.h"

// 按键0按下事件处理函数
void key0_pressed() {
  // 播放按键0的专属提示音 (1800Hz)
  buzzer_tone(1800, 200);

  // 在这里添加按键0的特定功能
  Serial.println("Key 0 specific function executed");

  // 可以在这里添加更多按键0的专属功能
  // 例如：切换模式、启动特定功能等
}
