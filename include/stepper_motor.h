#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include "hal.h"

// 步进电机参数定义
#define STEPS_PER_REVOLUTION 2048  // 28BYJ-48 每转步数 (64*32)
#define STEP_SEQUENCE_LENGTH 8     // 8步序列长度

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

// 步进电机状态结构体
typedef struct {
    int current_step;           // 当前步数位置
    motor_direction_t direction; // 转动方向
    motor_speed_t speed;        // 转动速度
    bool is_running;            // 是否正在运行
    unsigned long last_step_time; // 上次步进时间
    int target_steps;           // 目标步数
    int remaining_steps;        // 剩余步数
} stepper_motor_t;

// 函数声明
void stepper_motor_init();
void stepper_motor_set_speed(motor_speed_t speed);
void stepper_motor_set_direction(motor_direction_t direction);
void stepper_motor_rotate_angle(float angle);
void stepper_motor_rotate_steps(int steps);
void stepper_motor_start();
void stepper_motor_stop();
void stepper_motor_update();
bool stepper_motor_is_running();

// 低级控制函数
void stepper_motor_step();
void stepper_motor_set_pins(uint8_t step_pattern);

#endif // STEPPER_MOTOR_H
