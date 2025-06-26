#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "hal.h"

// 创建显示对象
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // -1 表示不使用复位引脚

// 蜂鸣器初始化函数
void buzzer_init() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

// 蜂鸣器播放音调函数
void buzzer_tone(int frequency, int duration) {
  if (frequency == 0) {
    // 静音
    digitalWrite(BUZZER_PIN, LOW);
    delay(duration);
    return;
  }

  int period = 1000000 / frequency; // 微秒
  int half_period = period / 2;
  int cycles = (long)frequency * duration / 1000;

  for (int i = 0; i < cycles; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(half_period);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(half_period);
  }
}

// 播放启动旋律函数
void play_startup_melody() {
  // 播放短促的3个上升音节
  buzzer_tone(1200, 100);
  delay(50);
  buzzer_tone(1500, 100);
  delay(50);
  buzzer_tone(2000, 100);
}

// 按键初始化函数
void keys_init() {
  // 设置 PE4 和 PE5 为输入（0）不开启上拉（0）
  DDRE &= ~((1 << KEY2_PIN) | (1 << KEY3_PIN));     // 输入模式
  // PORTE &= ~((1 << KEY2_PIN) | (1 << KEY3_PIN));    // 不开启上拉电阻

  // 设置 PD3 和 PD4 为输入（0）不开启上拉（0）
  DDRD &= ~((1 << KEY0_PIN) | (1 << KEY1_PIN));     // 输入模式
  // PORTD &= ~((1 << KEY0_PIN) | (1 << KEY1_PIN));    // 不开启上拉电阻
}

// 按键状态变量（默认低电平）
bool key0_last_state = LOW;
bool key1_last_state = LOW;
bool key2_last_state = LOW;
bool key3_last_state = LOW;

// 处理按键按下事件
void handle_key_press(int key_num) {
  // 清屏
  display.clearDisplay();

  // 设置文字属性
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 8);

  // 显示按键信息
  display.print("KEY");
  display.print(key_num);
  display.println(" PRESSED");

  // 更新显示
  display.display();

  // 播放1800Hz提示音
  buzzer_tone(1800, 200);

  Serial.print("Key ");
  Serial.print(key_num);
  Serial.println(" pressed");
}

// 检查按键状态
void check_keys() {
  // 使用寄存器直接读取按键状态
  bool key0_current = PIND & (1 << KEY0_PIN);
  bool key1_current = PIND & (1 << KEY1_PIN);
  bool key2_current = PINE & (1 << KEY2_PIN);
  bool key3_current = PINE & (1 << KEY3_PIN);

  Serial.print("key0: ");
  Serial.print(key0_current);
  Serial.print(". key1: ");
  Serial.print(key1_current);
  Serial.print("  key2: ");
  Serial.print(key2_current);
  Serial.print("  key3: ");
  Serial.println(key3_current);
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

void setup() {
  // 初始化串口调试（可选）
  Serial.begin(115200);



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

  // 初始化蜂鸣器
  buzzer_init();

  // 初始化按键
  keys_init();

  // 播放启动旋律
  play_startup_melody();

}

void loop() {
  // 检查按键状态
  check_keys();

  // 短暂延时，避免过于频繁的检测
  delay(10);
}
