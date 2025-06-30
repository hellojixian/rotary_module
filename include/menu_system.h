#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <Arduino.h>
#include "keys.h"
#include "config.h"
#include "camera.h"
#include "ui_display.h"

// 菜单状态枚举
typedef enum {
    MENU_STATE_STANDBY = 0,         // 待机状态
    MENU_STATE_CAMERA_MODE,         // 相机模式（相机连接时）
    MENU_STATE_SCAN_MODE,           // 3D扫描模式（相机未连接时）
    MENU_STATE_CONFIG,              // 配置模式
    MENU_STATE_CONFIG_EDIT,         // 配置编辑模式
    MENU_STATE_PHOTO_RUNNING,       // 拍照模式运行中
    MENU_STATE_SCAN_RUNNING,        // 3D扫描运行中
    MENU_STATE_COUNTDOWN            // 倒计时状态
} menu_state_t;

// 菜单系统状态结构体
typedef struct {
    menu_state_t current_state;
    menu_state_t previous_state;
    unsigned long state_enter_time;
    unsigned long last_update_time;
    bool state_changed;
} menu_system_state_t;

// 函数声明
void menu_init(void);
void menu_update(void);
menu_state_t menu_get_state(void);
void menu_set_state(menu_state_t new_state);
void menu_handle_key_events(void);

// 状态处理函数
void menu_handle_standby_state(void);
void menu_handle_camera_mode_state(void);
void menu_handle_scan_mode_state(void);
void menu_handle_config_state(void);
void menu_handle_config_edit_state(void);
void menu_handle_photo_running_state(void);
void menu_handle_scan_running_state(void);
void menu_handle_countdown_state(void);

// 按键处理函数
void menu_handle_key_press(key_num_t key, key_event_t event);
void menu_handle_cancel_key(key_event_t event);
void menu_handle_prev_key(key_event_t event);
void menu_handle_next_key(key_event_t event);
void menu_handle_ok_key(key_event_t event);

// 状态切换函数
void menu_enter_standby(void);
void menu_enter_camera_mode(void);
void menu_enter_scan_mode(void);
void menu_enter_config(void);
void menu_enter_config_edit(void);
void menu_exit_config_edit(void);
void menu_start_photo_mode(void);
void menu_start_scan_mode(void);
void menu_stop_running_mode(void);

// 辅助函数
void menu_update_display(void);
void menu_check_camera_status_change(void);
const char* menu_get_state_string(menu_state_t state);

#endif // MENU_SYSTEM_H
