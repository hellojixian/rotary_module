#include <Arduino.h>
#include "keys.h"
#include "buzzer.h"

// 按键2按下事件处理函数
void key2_pressed() {
  // 播放按键2的专属提示音 (2000Hz)
  buzzer_tone(2000, 200);

  // 在这里添加按键2的特定功能
  Serial.println("Key 2 specific function executed");

  // 可以在这里添加更多按键2的专属功能
  // 例如：保存设置、启动测量等
}
