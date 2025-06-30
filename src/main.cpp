#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "hal.h"
#include "buzzer.h"
#include "keys.h"
#include "voltage.h"
#include "stepper_motor.h"
#include "clock_verify.h"
#include "camera.h"
#include "config.h"
#include "menu_system.h"
#include "ui_display.h"
#include "photo_mode.h"
#include "scan_mode.h"

// 创建显示对象
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // -1 表示不使用复位引脚


void setup() {
  // 初始化串口调试（可选）
  Serial.begin(115200);


  verify_clock();

  // 初始化 I2C 总线（默认使用 Wire，PC4 = SDA, PC5 = SCL）
  Wire.begin();

  // 初始化 OLED 显示屏
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("OLED 初始化失败"));
    while (true); // 停止程序
  }

  // 清屏
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println(F("Initializing..."));
  display.display();

  // 初始化各个模块
  buzzer_init();
  keys_init();
  voltage_sensor_init();
  stepper_motor_init();
  camera_init();
  config_init();
  ui_init();
  menu_init();
  photo_mode_init();
  scan_mode_init();

  // 播放启动旋律
  play_startup_melody();

  // 初始读取电池电压
  battery_voltage = read_battery_voltage();


}

void loop() {
  // 更新按键状态
  keys_update();

  // 更新菜单系统（包含按键处理和状态管理）
  menu_update();

  // 更新步进电机状态
  stepper_motor_update();

  // 更新相机状态
  camera_update_status();

  // 更新相机触发状态（非阻塞）
  camera_update_triggers();

  // 更新拍照模式
  photo_mode_update();

  // 更新3D扫描模式
  scan_mode_update();

  // 更新电压读取（每2秒一次）
  update_voltage_reading();

  // 短暂延时以避免过度占用CPU
  delay(10);
}
