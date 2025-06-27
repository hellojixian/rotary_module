#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "keys.h"
#include "buzzer.h"

// 外部显示对象声明
extern Adafruit_SSD1306 display;

// 按键状态变量（默认低电平）
bool key0_last_state = LOW;
bool key1_last_state = LOW;
bool key2_last_state = LOW;
bool key3_last_state = LOW;

// 按键初始化函数
void keys_init() {
  // 设置 PE4 和 PE5 为输入（0）不开启上拉（0）
  DDRE &= ~((1 << KEY2_PIN) | (1 << KEY3_PIN));     // 输入模式
  // PORTE &= ~((1 << KEY2_PIN) | (1 << KEY3_PIN));    // 不开启上拉电阻

  // 设置 PD3 和 PD4 为输入（0）不开启上拉（0）
  DDRD &= ~((1 << KEY0_PIN) | (1 << KEY1_PIN));     // 输入模式
  // PORTD &= ~((1 << KEY0_PIN) | (1 << KEY1_PIN));    // 不开启上拉电阻
}

// 处理按键按下事件
void handle_key_press(int key_num) {
  // 清屏
  display.clearDisplay();

  // 设置文字属性
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 8);

  // 显示按键信息
  display.print(F("KEY"));
  display.print(key_num);
  display.println(F(" PRESSED"));

  // 更新显示
  display.display();

  // 调用对应的按键事件处理函数
  switch(key_num) {
    case 0:
      key0_pressed();
      break;
    case 1:
      key1_pressed();
      break;
    case 2:
      key2_pressed();
      break;
    case 3:
      key3_pressed();
      break;
  }

  Serial.print(F("Key "));
  Serial.print(key_num);
  Serial.println(F(" pressed"));
}

// 检查按键状态
void check_keys() {
  // 使用寄存器直接读取按键状态
  bool key0_current = PIND & (1 << KEY0_PIN);
  bool key1_current = PIND & (1 << KEY1_PIN);
  bool key2_current = PINE & (1 << KEY2_PIN);
  bool key3_current = PINE & (1 << KEY3_PIN);

  // 检测按键0（上升沿触发）
  if (key0_last_state == LOW && key0_current == HIGH) {
    handle_key_press(0);
  }
  key0_last_state = key0_current;

  // 检测按键1（上升沿触发）
  if (key1_last_state == LOW && key1_current == HIGH) {
    handle_key_press(1);
  }
  key1_last_state = key1_current;

  // 检测按键2（上升沿触发）
  if (key2_last_state == LOW && key2_current == HIGH) {
    handle_key_press(2);
  }
  key2_last_state = key2_current;

  // 检测按键3（上升沿触发）
  if (key3_last_state == LOW && key3_current == HIGH) {
    handle_key_press(3);
  }
  key3_last_state = key3_current;
}
