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

  // 设置文字颜色
  display.setTextColor(SSD1306_WHITE);

  // 设置文字大小（1~3）
  display.setTextSize(1);

  // 设置光标位置
  display.setCursor(0, 10);

  // 打印文本
  display.println(F("Hello, World! Buzzer"));

  // 显示内容
  display.display();

  // 初始化各个模块
  buzzer_init();
  keys_init();
  voltage_sensor_init();
  stepper_motor_init();

  // 播放启动旋律
  play_startup_melody();

  // 初始读取电池电压
  battery_voltage = read_battery_voltage();

  Serial.println("System initialized successfully!");
}

void loop() {
  // 检查按键状态
  check_keys();

  // 更新步进电机状态
  stepper_motor_update();

  // 更新电压显示
  update_voltage_display();

  // 短暂延时，避免过于频繁的检测
  delay(10);
}
