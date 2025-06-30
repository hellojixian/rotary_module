#ifndef CAMERA_H
#define CAMERA_H

#include <Arduino.h>
#include "hal.h"



// 相机连接状态枚举
typedef enum {
    CAMERA_DISCONNECTED = 0,    // 完全未连接
    CAMERA_CABLE_CONNECTED = 1, // 连接线已链接但未检测到相机
    CAMERA_FULLY_CONNECTED = 2  // 已经正常链接到相机
} camera_status_t;

// 触发状态枚举
typedef enum {
    TRIGGER_IDLE = 0,           // 空闲状态
    TRIGGER_FOCUS_ACTIVE = 1,   // 对焦触发激活
    TRIGGER_SHUTTER_ACTIVE = 2  // 快门触发激活
} trigger_state_t;

// 相机状态结构体
typedef struct {
    camera_status_t status;
    bool cable_connected;
    bool camera_detected;
    bool trigger_sensor_last_state;
    bool focus_trigger_last_state;
    unsigned long last_status_check;

    // 非阻塞触发管理
    trigger_state_t trigger_state;
    unsigned long trigger_start_time;
    unsigned long trigger_duration;
} camera_state_t;

// 函数声明
void camera_init(void);
void camera_update_status(void);
void camera_update_triggers(void);  // 新增：更新触发状态
camera_status_t camera_get_status(void);
const char* camera_get_status_string(void);
void camera_display_status(void);

// 相机触发功能（低电平触发，默认高电平INPUT）
void camera_trigger_focus(void);
void camera_trigger_shutter(void);
void camera_release_triggers(void);

// 内部状态检测函数
bool camera_check_cable_connection(void);
bool camera_check_camera_detection(void);

#endif // CAMERA_H
