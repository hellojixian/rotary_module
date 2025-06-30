#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "camera.h"

// 外部显示对象声明
extern Adafruit_SSD1306 display;

// 相机状态变量
static camera_state_t camera_state;

/**
 * 初始化相机模块
 */
void camera_init(void) {
    // 设置 CAMERA_TRIGGER_SENSOR_PIN (PD2) 为输入模式，开启上拉电阻
    // 当连接线插入时，该引脚会被拉低
    DDRD &= ~(1 << CAMERA_TRIGGER_SENSOR_PIN);  // 设置为输入模式
    PORTD |= (1 << CAMERA_TRIGGER_SENSOR_PIN);  // 开启上拉电阻

    // 设置 CAMERA_FOCUS_TRIGGER_PIN (PC1) 为输入模式，开启上拉电阻
    // 默认为高电平INPUT模式，触发时输出低电平
    DDRC &= ~(1 << CAMERA_FOCUS_TRIGGER_PIN);   // 设置为输入模式
    PORTC |= (1 << CAMERA_FOCUS_TRIGGER_PIN);   // 开启上拉电阻，默认高电平

    // 设置 CAMERA_SHUTTER_TRIGGER_PIN (PC0) 为输入模式，开启上拉电阻
    // 默认为高电平INPUT模式，触发时输出低电平
    DDRC &= ~(1 << CAMERA_SHUTTER_TRIGGER_PIN); // 设置为输入模式
    PORTC |= (1 << CAMERA_SHUTTER_TRIGGER_PIN); // 开启上拉电阻，默认高电平

    // 初始化状态结构体
    camera_state.status = CAMERA_DISCONNECTED;
    camera_state.cable_connected = false;
    camera_state.camera_detected = false;
    camera_state.trigger_sensor_last_state = true;  // 上拉状态下默认为高电平
    camera_state.focus_trigger_last_state = true;   // 默认为高电平（上拉状态）
    camera_state.last_status_check = 0;

    // 初始化触发状态
    camera_state.trigger_state = TRIGGER_IDLE;
    camera_state.trigger_start_time = 0;
    camera_state.trigger_duration = 0;

    Serial.println(F("Camera module initialized"));
}

/**
 * 检查连接线是否已插入
 * 当 CAMERA_TRIGGER_SENSOR_PIN 从 INPUT_PULLUP 变为低电平时，说明连接线已插入
 */
bool camera_check_cable_connection(void) {
    // 读取触发传感器引脚状态
    bool current_state = (PIND & (1 << CAMERA_TRIGGER_SENSOR_PIN)) != 0;

    // 如果从高电平变为低电平，说明连接线已插入
    if (camera_state.trigger_sensor_last_state && !current_state) {
        camera_state.cable_connected = true;
        Serial.println(F("Camera cable connected"));
    }
    // 如果从低电平变为高电平，说明连接线已拔出
    else if (!camera_state.trigger_sensor_last_state && current_state) {
        camera_state.cable_connected = false;
        camera_state.camera_detected = false;  // 连接线拔出时，相机检测也失效
        Serial.println(F("Camera cable disconnected"));
    }

    camera_state.trigger_sensor_last_state = current_state;
    return camera_state.cable_connected;
}

/**
 * 检查是否检测到相机
 * 当连接线插入后，CAMERA_FOCUS_TRIGGER_PIN 从低电平变为高电平时，说明检测到相机
 */
bool camera_check_camera_detection(void) {
    // 只有在连接线已插入的情况下才检测相机
    if (!camera_state.cable_connected) {
        camera_state.camera_detected = false;
        return false;
    }

    // 读取焦点触发引脚状态
    bool current_state = (PINC & (1 << CAMERA_FOCUS_TRIGGER_PIN)) != 0;

    // 直接根据当前状态判断相机是否连接
    // 高电平 = 相机连接，低电平 = 相机未连接
    if (current_state && !camera_state.camera_detected) {
        camera_state.camera_detected = true;
        Serial.println(F("Camera detected"));
    }
    else if (!current_state && camera_state.camera_detected) {
        camera_state.camera_detected = false;
        Serial.println(F("Camera connection lost"));
    }

    camera_state.focus_trigger_last_state = current_state;
    return camera_state.camera_detected;
}

/**
 * 更新相机状态
 * 需要在主循环中定期调用
 */
void camera_update_status(void) {
    unsigned long current_time = millis();

    // 每100ms检查一次状态
    if (current_time - camera_state.last_status_check >= 100) {
        camera_state.last_status_check = current_time;

        // 检查连接线状态
        bool cable_connected = camera_check_cable_connection();

        // 检查相机检测状态
        bool camera_detected = camera_check_camera_detection();

        // 更新总体状态
        camera_status_t new_status;
        if (!cable_connected) {
            new_status = CAMERA_DISCONNECTED;
        } else if (cable_connected && !camera_detected) {
            new_status = CAMERA_CABLE_CONNECTED;
        } else {
            new_status = CAMERA_FULLY_CONNECTED;
        }

        // 如果状态发生变化，输出日志
        if (new_status != camera_state.status) {
            camera_state.status = new_status;
            Serial.print(F("Camera status changed to: "));
            Serial.println(camera_get_status_string());
        }
    }
}

/**
 * 更新触发状态（非阻塞）
 * 需要在主循环中调用
 */
void camera_update_triggers(void) {
    if (camera_state.trigger_state == TRIGGER_IDLE) {
        return;  // 没有活动的触发，直接返回
    }

    unsigned long current_time = millis();

    // 检查触发时间是否已到
    if (current_time - camera_state.trigger_start_time >= camera_state.trigger_duration) {
        // 触发时间结束，释放触发信号
        camera_release_triggers();
        camera_state.trigger_state = TRIGGER_IDLE;

        Serial.println(F("Camera trigger completed"));
    }
}

/**
 * 获取当前相机状态
 */
camera_status_t camera_get_status(void) {
    return camera_state.status;
}

/**
 * 获取相机状态字符串
 */
const char* camera_get_status_string(void) {
    switch (camera_state.status) {
        case CAMERA_DISCONNECTED:
            return "Disconnected";
        case CAMERA_CABLE_CONNECTED:
            return "Cable Connected";
        case CAMERA_FULLY_CONNECTED:
            return "Fully Connected";
        default:
            return "Unknown";
    }
}

/**
 * 在显示屏上显示相机状态
 */
void camera_display_status(void) {
    // 在屏幕左下角显示相机状态
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 24);

    // display.print(F("CAM: "));
    switch (camera_state.status) {
        case CAMERA_DISCONNECTED:
            display.print(F("No Camera"));
            break;
        case CAMERA_CABLE_CONNECTED:
            display.print(F("Camera Cable Connected"));
            break;
        case CAMERA_FULLY_CONNECTED:
            display.print(F("Camera Connected"));
            break;
    }
}

/**
 * 触发相机对焦（非阻塞）
 * 将 CAMERA_FOCUS_TRIGGER_PIN 临时设置为输出低电平
 */
void camera_trigger_focus(void) {
    if (camera_state.status != CAMERA_FULLY_CONNECTED) {
        Serial.println(F("Camera not ready for focus trigger"));
        return;
    }

    if (camera_state.trigger_state != TRIGGER_IDLE) {
        Serial.println(F("Camera trigger already active"));
        return;
    }

    // 设置为输出模式并输出低电平（触发）
    DDRC |= (1 << CAMERA_FOCUS_TRIGGER_PIN);
    PORTC &= ~(1 << CAMERA_FOCUS_TRIGGER_PIN);

    // 设置触发状态
    camera_state.trigger_state = TRIGGER_FOCUS_ACTIVE;
    camera_state.trigger_start_time = millis();
    camera_state.trigger_duration = CAMERA_FOCUS_TRIGGER_TIME;

    Serial.println(F("Camera focus triggered (non-blocking)"));
}

/**
 * 触发相机快门（非阻塞）
 * 将 CAMERA_SHUTTER_TRIGGER_PIN 设置为输出低电平
 */
void camera_trigger_shutter(void) {
    if (camera_state.status != CAMERA_FULLY_CONNECTED) {
        Serial.println(F("Camera not ready for shutter trigger"));
        return;
    }

    if (camera_state.trigger_state != TRIGGER_IDLE) {
        Serial.println(F("Camera trigger already active"));
        return;
    }

    // 设置为输出模式并输出低电平（触发）
    DDRC |= (1 << CAMERA_SHUTTER_TRIGGER_PIN);
    PORTC &= ~(1 << CAMERA_SHUTTER_TRIGGER_PIN);

    // 设置触发状态
    camera_state.trigger_state = TRIGGER_SHUTTER_ACTIVE;
    camera_state.trigger_start_time = millis();
    camera_state.trigger_duration = CAMERA_SHUTTER_TRIGGER_TIME;

    Serial.println(F("Camera shutter triggered (non-blocking)"));
}

/**
 * 释放所有触发信号
 * 将触发引脚恢复为输入模式（高电平上拉状态）
 */
void camera_release_triggers(void) {
    // 将 CAMERA_FOCUS_TRIGGER_PIN 恢复为输入模式，开启上拉电阻
    DDRC &= ~(1 << CAMERA_FOCUS_TRIGGER_PIN);
    PORTC |= (1 << CAMERA_FOCUS_TRIGGER_PIN);

    // 将 CAMERA_SHUTTER_TRIGGER_PIN 恢复为输入模式，开启上拉电阻
    DDRC &= ~(1 << CAMERA_SHUTTER_TRIGGER_PIN);
    PORTC |= (1 << CAMERA_SHUTTER_TRIGGER_PIN);

    Serial.println(F("Camera triggers released"));
}
