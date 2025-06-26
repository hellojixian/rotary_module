#include <Arduino.h>
#include "keys.h"
#include "buzzer.h"

// 按键1按下事件处理函数
void key1_pressed() {
  // 播放按键1的专属提示音 (1900Hz)
  buzzer_tone(1900, 200);

  // 在这里添加按键1的特定功能
  Serial.println("Key 1 specific function executed");

  // 可以在这里添加更多按键1的专属功能
  // 例如：调节参数、切换显示等
}
