#ifndef SCAN_MODE_H
#define SCAN_MODE_H

#include <Arduino.h>
#include "config.h"
#include "stepper_motor.h"
#include "buzzer.h"
#include "ui_display.h"

// 3D扫描模式状态枚举
typedef enum {
    SCAN_STATE_IDLE = 0,            // 空闲状态
    SCAN_STATE_COUNTDOWN,           // 倒计时状态
    SCAN_STATE_RUNNING,             // 运行状态
    SCAN_STATE_STOPPED              // 停止状态
} scan_state_t;

// 3D扫描模式状态结构体
typedef struct {
    scan_state_t current_state;
    unsigned long state_enter_time;
    unsigned long last_update_time;
    
    // 倒计时相关
    uint8_t countdown_seconds;
    unsigned long last_beep_time;
    
    // 运行统计
    unsigned long start_time;
    unsigned long total_runtime;
    uint32_t total_steps;
    float total_turns;
    
    // 显示更新
    unsigned long last_display_update;
    
} scan_mode_state_t;

// 函数声明
void scan_mode_init(void);
void scan_mode_start(void);
void scan_mode_stop(void);
void scan_mode_update(void);
bool scan_mode_is_running(void);
scan_state_t scan_mode_get_state(void);

// 状态处理函数
void scan_mode_handle_countdown(void);
void scan_mode_handle_running(void);

// 辅助函数
void scan_mode_start_countdown(void);
void scan_mode_start_scanning(void);
void scan_mode_update_statistics(void);
void scan_mode_update_display(void);

// 统计计算函数
float scan_mode_calculate_turns(void);
unsigned long scan_mode_get_elapsed_seconds(void);

#endif // SCAN_MODE_H
