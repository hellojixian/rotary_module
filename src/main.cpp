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
  buzzer_tone(1000, 100);
  delay(50);
  buzzer_tone(1500, 100);
  delay(50);
  buzzer_tone(2000, 100);
}

void setup() {
  digitalWrite(BUZZER_PIN, LOW);

  // 初始化串口调试（可选）
  Serial.begin(9600);



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

  // 播放启动旋律
  play_startup_melody();

}

void loop() {
  // 不做其他事情
}
