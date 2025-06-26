#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "voltage.h"

// 外部显示对象声明
extern Adafruit_SSD1306 display;

// 电压监测相关变量
unsigned long last_voltage_check = 0;
float battery_voltage = 0.0;

// 电压传感器初始化函数
void voltage_sensor_init() {
  // PC2引脚默认为模拟输入，无需特殊设置
}

// 读取电池电压函数
float read_battery_voltage() {
  // 读取ADC值 (0-1023)
  int adc_value = analogRead(VOLTAGE_SENSOR_PIN);

  // 计算实际电压
  // ADC参考电压为5V，10位ADC (0-1023)
  // 由于使用10K+10K分压器，实际电压是测量值的2倍
  float voltage = (adc_value * 5.0 / 1023.0) * 2.0;

  return voltage;
}

// 显示电池电压函数
void display_battery_voltage() {
  // 在屏幕右上角显示电压
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(85, 0);  // 右上角位置
  display.print(battery_voltage, 1);  // 显示一位小数
  display.print("V");
}

// 更新电压显示函数
void update_voltage_display() {
  // 每2秒读取一次电池电压
  unsigned long current_time = millis();
  if (current_time - last_voltage_check >= 2000) {
    battery_voltage = read_battery_voltage();
    last_voltage_check = current_time;

    // 更新显示（如果没有按键被按下）
    if (!((PIND & (1 << KEY0_PIN)) || (PIND & (1 << KEY1_PIN)) ||
          (PINE & (1 << KEY2_PIN)) || (PINE & (1 << KEY3_PIN)))) {
      display.clearDisplay();

      // 显示主界面文本
      display.setTextColor(SSD1306_WHITE);
      display.setTextSize(1);
      display.setCursor(0, 10);
      display.println(F("Hello, World! Buzzer"));

      // 显示电池电压
      display_battery_voltage();

      display.display();
    }

    // 串口输出电压信息
    Serial.print("Battery Voltage: ");
    Serial.print(battery_voltage, 2);
    Serial.println("V");
  }
}
