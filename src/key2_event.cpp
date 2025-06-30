#include <Arduino.h>
#include "keys.h"
#include "buzzer.h"
#include "camera.h"

// 按键2按下事件处理函数
void key2_pressed() {
  // 播放按键2的专属提示音 (2000Hz)
  buzzer_tone(2000, 200);

  // 按键2功能：相机对焦控制
  Serial.println(F("Key 2 pressed: Camera focus control"));

  if (camera_get_status() == CAMERA_FULLY_CONNECTED) {
    // 触发对焦（非阻塞）
    camera_trigger_focus();
  } else {
    Serial.println(F("Camera not ready for focus"));
  }
}
