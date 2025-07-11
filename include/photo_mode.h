#ifndef PHOTO_MODE_H
#define PHOTO_MODE_H

#include <Arduino.h>
#include "config.h"
#include "camera.h"
#include "stepper_motor.h"
#include "buzzer.h"
#include "ui_display.h"

// 角度补偿参数
#define ANGLE_COMPENSATION_BASE 0        // 基础补偿步数
#define ANGLE_COMPENSATION_PER_STOP 4     // 每次暂停的补偿步数

// 拍照模式状态枚举
typedef enum {
    PHOTO_STATE_IDLE = 0,           // 空闲状态
    PHOTO_STATE_COUNTDOWN,          // 倒计时状态
    PHOTO_STATE_FOCUS,              // 对焦状态
    PHOTO_STATE_PRE_FIRST_SHOT,     // 第一张照片前停留
    PHOTO_STATE_FIRST_SHOT,         // 拍摄第一张照片
    PHOTO_STATE_POST_FIRST_SHOT,    // 第一张照片后停留
    PHOTO_STATE_ROTATING,           // 旋转电机
    PHOTO_STATE_PRE_SHOOTING,       // 拍摄前停留
    PHOTO_STATE_SHOOTING,           // 拍摄照片
    PHOTO_STATE_POST_SHOOTING,      // 拍摄后停留
    PHOTO_STATE_COMPLETE,           // 完成状态
    PHOTO_STATE_STOPPED             // 停止状态
} photo_state_t;

// 拍照模式状态结构体
typedef struct {
    photo_state_t current_state;
    unsigned long state_enter_time;
    unsigned long last_update_time;

    // 倒计时相关
    uint8_t countdown_seconds;
    unsigned long last_beep_time;

    // 拍照进度相关
    uint8_t total_photos;
    uint8_t current_photo;
    uint16_t target_angle;
    uint16_t current_angle;
    uint16_t angle_per_photo;

    // 电机控制相关
    uint32_t steps_per_photo;
    uint32_t total_steps_moved;
    uint32_t compensation_steps;      // 角度补偿步数

    // 相机触发相关
    unsigned long focus_start_time;
    unsigned long shutter_start_time;
    bool focus_triggered;
    bool shutter_triggered;

    // 显示更新相关
    unsigned long last_display_update;

} photo_mode_state_t;

// 函数声明
void photo_mode_init(void);
void photo_mode_start(void);
void photo_mode_stop(void);
void photo_mode_update(void);
bool photo_mode_is_running(void);
photo_state_t photo_mode_get_state(void);

// 状态处理函数
void photo_mode_handle_countdown(void);
void photo_mode_handle_focus(void);
void photo_mode_handle_pre_first_shot(void);
void photo_mode_handle_first_shot(void);
void photo_mode_handle_post_first_shot(void);
void photo_mode_handle_rotating(void);
void photo_mode_handle_pre_shooting(void);
void photo_mode_handle_shooting(void);
void photo_mode_handle_post_shooting(void);
void photo_mode_handle_complete(void);

// 辅助函数
void photo_mode_calculate_parameters(void);
void photo_mode_start_countdown(void);
void photo_mode_trigger_focus(void);
void photo_mode_trigger_shutter(void);
void photo_mode_start_rotation(void);
void photo_mode_finish_session(void);
void photo_mode_update_display(void);

// 角度和步数转换函数
uint32_t photo_mode_angle_to_steps(uint16_t angle);
uint16_t photo_mode_steps_to_angle(uint32_t steps);

#endif // PHOTO_MODE_H
