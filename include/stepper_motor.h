#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include "hal.h"

// 步进电机参数定义
// #define STEPS_PER_REVOLUTION_HALF 4076  // 28BYJ-48 每转步数 (半步模式)
// #define STEPS_PER_REVOLUTION_FULL 2038  // 28BYJ-48 每转步数 (全步模式)
#define STEPS_PER_REVOLUTION_HALF 4096  // 28BYJ-48 每转步数 (半步模式)
#define STEPS_PER_REVOLUTION_FULL 2048  // 28BYJ-48 每转步数 (全步模式)
#define STEP_SEQUENCE_LENGTH_HALF 8     // 半步序列长度
#define STEP_SEQUENCE_LENGTH_FULL 4     // 全步序列长度

// 转动方向定义
typedef enum {
    CLOCKWISE = 1,
    COUNTER_CLOCKWISE = -1
} motor_direction_t;

// 速度等级定义
typedef enum {
    SPEED_LOW = 0,
    SPEED_HIGH = 1
} motor_speed_t;

// 步进模式定义
typedef enum {
    STEP_MODE_HALF = 0,    // 半步模式（平滑，扭矩较小）
    STEP_MODE_FULL = 1     // 全步模式（扭矩大）
} step_mode_t;

// 步进电机状态结构体
typedef struct {
    int current_step;           // 当前步数位置
    motor_direction_t direction; // 转动方向
    motor_speed_t speed;        // 转动速度
    step_mode_t step_mode;      // 步进模式
    bool is_running;            // 是否正在运行
    unsigned long last_step_time; // 上次步进时间
    int target_steps;           // 目标步数
    int remaining_steps;        // 剩余步数
} stepper_motor_t;

// 函数声明
void stepper_motor_init();
void stepper_motor_set_speed(motor_speed_t speed);
void stepper_motor_set_custom_speed(uint8_t delay_ms);
void stepper_motor_set_direction(motor_direction_t direction);
void stepper_motor_set_step_mode(step_mode_t mode);
void stepper_motor_rotate_angle(float angle);
void stepper_motor_rotate_steps(int steps);
void stepper_motor_start();
void stepper_motor_stop();
void stepper_motor_update();
bool stepper_motor_is_running();
step_mode_t stepper_motor_get_step_mode();
uint32_t stepper_motor_get_step_count();
void stepper_motor_reset_step_count();

// 扭矩优化函数
void stepper_motor_enable_high_torque();
void stepper_motor_disable_high_torque();

// 发热控制函数
void stepper_motor_enable_low_heat_mode();

// 低级控制函数
void stepper_motor_step();
void stepper_motor_set_pins(uint8_t step_pattern);

#endif // STEPPER_MOTOR_H
