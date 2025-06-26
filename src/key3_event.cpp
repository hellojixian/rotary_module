#include <Arduino.h>
#include "keys.h"
#include "buzzer.h"

// 按键3按下事件处理函数
void key3_pressed() {
  // 播放按键3的专属提示音 (2100Hz)
  buzzer_tone(2100, 200);

  // 在这里添加按键3的特定功能
  Serial.println("Key 3 specific function executed");

  // 可以在这里添加更多按键3的专属功能
  // 例如：重置系统、进入配置模式等
}
