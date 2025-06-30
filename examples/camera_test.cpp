/**
 * 相机状态检测系统测试程序
 * 
 * 本程序用于测试相机状态检测功能，包括：
 * 1. 连接线插入检测
 * 2. 相机连接检测
 * 3. 状态显示
 * 4. 相机触发功能
 * 
 * 使用方法：
 * 1. 将此文件内容替换到 src/main.cpp
 * 2. 编译并上传到设备
 * 3. 打开串口监视器观察输出
 * 4. 测试连接线插拔和相机连接
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "hal.h"
#include "buzzer.h"
#include "camera.h"

// 创建显示对象
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
    // 初始化串口调试
    Serial.begin(115200);
    Serial.println(F("=== Camera Status Test ==="));

    // 初始化 I2C 总线
    Wire.begin();

    // 初始化 OLED 显示屏
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println(F("OLED 初始化失败"));
        while (true);
    }

    // 清屏并设置基本属性
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    // 初始化模块
    buzzer_init();
    camera_init();

    // 播放启动提示音
    buzzer_tone(1000, 200);
    delay(100);
    buzzer_tone(1500, 200);

    Serial.println(F("Camera test initialized"));
    Serial.println(F("Test instructions:"));
    Serial.println(F("1. Insert/remove camera cable"));
    Serial.println(F("2. Connect/disconnect camera"));
    Serial.println(F("3. Watch status changes"));
    Serial.println(F("========================"));
}

void loop() {
    static unsigned long last_display_update = 0;
    static camera_status_t last_status = CAMERA_DISCONNECTED;
    
    // 更新相机状态
    camera_update_status();
    
    // 检查状态变化
    camera_status_t current_status = camera_get_status();
    if (current_status != last_status) {
        // 状态发生变化，播放提示音
        switch (current_status) {
            case CAMERA_DISCONNECTED:
                buzzer_tone(800, 100);  // 低音表示断开
                break;
            case CAMERA_CABLE_CONNECTED:
                buzzer_tone(1200, 100); // 中音表示连接线插入
                break;
            case CAMERA_FULLY_CONNECTED:
                buzzer_tone(1600, 100); // 高音表示相机连接
                delay(50);
                buzzer_tone(1600, 100); // 双响表示完全连接
                break;
        }
        last_status = current_status;
    }
    
    // 每500ms更新一次显示
    unsigned long current_time = millis();
    if (current_time - last_display_update >= 500) {
        last_display_update = current_time;
        
        // 清屏
        display.clearDisplay();
        
        // 显示标题
        display.setCursor(0, 0);
        display.println(F("Camera Status Test"));
        
        // 显示当前状态
        display.setCursor(0, 12);
        display.print(F("Status: "));
        display.println(camera_get_status_string());
        
        // 显示相机状态图标
        camera_display_status();
        
        // 显示GPIO状态（用于调试）
        display.setCursor(0, 24);
        display.print(F("PD2:"));
        display.print((PIND & (1 << CAMERA_TRIGGER_SENSOR_PIN)) ? "H" : "L");
        display.print(F(" PC1:"));
        display.print((PINC & (1 << CAMERA_FOCUS_TRIGGER_PIN)) ? "H" : "L");
        
        // 更新显示
        display.display();
        
        // 串口输出详细状态
        Serial.print(F("Status: "));
        Serial.print(camera_get_status_string());
        Serial.print(F(" | PD2: "));
        Serial.print((PIND & (1 << CAMERA_TRIGGER_SENSOR_PIN)) ? "HIGH" : "LOW");
        Serial.print(F(" | PC1: "));
        Serial.println((PINC & (1 << CAMERA_FOCUS_TRIGGER_PIN)) ? "HIGH" : "LOW");
    }
    
    // 简单的按键模拟（如果有按键连接）
    // 这里可以添加按键检测来测试相机触发功能
    
    delay(10);
}

/**
 * 测试相机触发功能的辅助函数
 * 可以通过串口命令或按键调用
 */
void test_camera_triggers() {
    if (camera_get_status() == CAMERA_FULLY_CONNECTED) {
        Serial.println(F("Testing camera focus..."));
        camera_trigger_focus();
        delay(500);
        camera_release_triggers();
        
        delay(1000);
        
        Serial.println(F("Testing camera shutter..."));
        camera_trigger_focus();
        delay(500);
        camera_trigger_shutter();
        delay(100);
        camera_release_triggers();
        
        Serial.println(F("Camera trigger test completed"));
    } else {
        Serial.println(F("Camera not ready for trigger test"));
    }
}

/**
 * 手动GPIO状态检查函数
 * 用于调试GPIO配置是否正确
 */
void check_gpio_configuration() {
    Serial.println(F("=== GPIO Configuration Check ==="));
    
    // 检查PD2配置
    Serial.print(F("PD2 (CAMERA_TRIGGER_SENSOR_PIN): "));
    Serial.print(F("DDR="));
    Serial.print((DDRD & (1 << CAMERA_TRIGGER_SENSOR_PIN)) ? "OUT" : "IN");
    Serial.print(F(", PORT="));
    Serial.print((PORTD & (1 << CAMERA_TRIGGER_SENSOR_PIN)) ? "HIGH" : "LOW");
    Serial.print(F(", PIN="));
    Serial.println((PIND & (1 << CAMERA_TRIGGER_SENSOR_PIN)) ? "HIGH" : "LOW");
    
    // 检查PC1配置
    Serial.print(F("PC1 (CAMERA_FOCUS_TRIGGER_PIN): "));
    Serial.print(F("DDR="));
    Serial.print((DDRC & (1 << CAMERA_FOCUS_TRIGGER_PIN)) ? "OUT" : "IN");
    Serial.print(F(", PORT="));
    Serial.print((PORTC & (1 << CAMERA_FOCUS_TRIGGER_PIN)) ? "HIGH" : "LOW");
    Serial.print(F(", PIN="));
    Serial.println((PINC & (1 << CAMERA_FOCUS_TRIGGER_PIN)) ? "HIGH" : "LOW");
    
    // 检查PC0配置
    Serial.print(F("PC0 (CAMERA_SHUTTER_TRIGGER_PIN): "));
    Serial.print(F("DDR="));
    Serial.print((DDRC & (1 << CAMERA_SHUTTER_TRIGGER_PIN)) ? "OUT" : "IN");
    Serial.print(F(", PORT="));
    Serial.print((PORTC & (1 << CAMERA_SHUTTER_TRIGGER_PIN)) ? "HIGH" : "LOW");
    Serial.print(F(", PIN="));
    Serial.println((PINC & (1 << CAMERA_SHUTTER_TRIGGER_PIN)) ? "HIGH" : "LOW");
    
    Serial.println(F("================================"));
}
