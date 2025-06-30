#include <Arduino.h>
#include "keys.h"
#include "buzzer.h"
#include "camera.h"

// 按键3按下事件处理函数
void key3_pressed() {
  // 播放按键3的专属提示音 (2100Hz)
  buzzer_tone(2100, 200);

  // 按键3功能：相机对焦控制
  Serial.println(F("Key 3 pressed: Camera focus control"));

  if (camera_get_status() == CAMERA_FULLY_CONNECTED) {
    // 触发对焦
    camera_trigger_shutter();
  } else {
    Serial.println(F("Camera not ready for focus"));
  }
}
