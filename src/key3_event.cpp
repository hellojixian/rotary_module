#include <Arduino.h>
#include "keys.h"
#include "buzzer.h"
#include "camera.h"

// 按键3按下事件处理函数
void key3_pressed() {
  // 按键3功能：相机对焦控制
  Serial.println(F("Key 3 pressed: Camera focus control"));

  // 检查相机连接状态
  if (camera_get_status() != CAMERA_FULLY_CONNECTED) {
    // 播放错误提示音
    buzzer_tone(1400, 200);
    Serial.println(F("Camera not ready for focus"));
    return;
  }

  // 检查触发状态是否空闲
  if (!camera_is_trigger_idle()) {
    // 播放错误提示音
    buzzer_tone(1400, 200);
    Serial.println(F("Camera trigger already active"));
    return;
  }

  // 触发对焦（非阻塞）
  camera_trigger_focus();

  // 播放成功提示音
  buzzer_tone(2100, 200);
}
