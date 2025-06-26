#include <Arduino.h>
#include "stepper_motor.h"

// 步进电机全局状态
static stepper_motor_t motor_state;

// 速度延时配置 (微秒)
static const unsigned long speed_delays[] = {
    3000,  // SPEED_LOW: 3ms延时
    1000   // SPEED_HIGH: 1ms延时
};

// 28BYJ-48 8步序列 (半步模式，更平滑)
static const uint8_t step_sequence[STEP_SEQUENCE_LENGTH] = {
    0b0001,  // 0001
    0b0011,  // 0011
    0b0010,  // 0010
    0b0110,  // 0110
    0b0100,  // 0100
    0b1100,  // 1100
    0b1000,  // 1000
    0b1001   // 1001
};

/**
 * 初始化步进电机
 */
void stepper_motor_init() {
    // 启用PE0和PE2引脚 (根据用户提供的代码)
    MCUSR = 0xff;

    // 设置PE0-PE3为输出模式
    DDRE |= (1 << PE0) | (1 << PE1) | (1 << PE2) | (1 << PE3);

    // 初始化所有引脚为低电平
    PORTE &= ~((1 << PE0) | (1 << PE1) | (1 << PE2) | (1 << PE3));

    // 初始化电机状态
    motor_state.current_step = 0;
    motor_state.direction = CLOCKWISE;
    motor_state.speed = SPEED_LOW;
    motor_state.is_running = false;
    motor_state.last_step_time = 0;
    motor_state.target_steps = 0;
    motor_state.remaining_steps = 0;

    Serial.println("Stepper motor initialized");
}

/**
 * 设置电机速度
 */
void stepper_motor_set_speed(motor_speed_t speed) {
    motor_state.speed = speed;
    Serial.print("Motor speed set to: ");
    Serial.println(speed == SPEED_LOW ? "LOW" : "HIGH");
}

/**
 * 设置电机转动方向
 */
void stepper_motor_set_direction(motor_direction_t direction) {
    motor_state.direction = direction;
    Serial.print("Motor direction set to: ");
    Serial.println(direction == CLOCKWISE ? "CLOCKWISE" : "COUNTER_CLOCKWISE");
}

/**
 * 根据角度旋转电机
 * @param angle 角度 (度)，正数顺时针，负数逆时针
 */
void stepper_motor_rotate_angle(float angle) {
    // 计算需要的步数
    int steps = (int)((angle / 360.0) * STEPS_PER_REVOLUTION);

    // 根据角度符号设置方向
    if (steps < 0) {
        motor_state.direction = COUNTER_CLOCKWISE;
        steps = -steps;
    } else {
        motor_state.direction = CLOCKWISE;
    }

    stepper_motor_rotate_steps(steps);

    Serial.print("Rotating ");
    Serial.print(angle);
    Serial.print(" degrees (");
    Serial.print(steps);
    Serial.println(" steps)");
}

/**
 * 旋转指定步数
 */
void stepper_motor_rotate_steps(int steps) {
    if (steps <= 0) return;

    motor_state.target_steps = steps;
    motor_state.remaining_steps = steps;
    motor_state.is_running = true;
    motor_state.last_step_time = micros();

    Serial.print("Starting rotation: ");
    Serial.print(steps);
    Serial.println(" steps");
}

/**
 * 启动电机连续转动
 */
void stepper_motor_start() {
    motor_state.is_running = true;
    motor_state.remaining_steps = -1; // -1表示连续转动
    motor_state.last_step_time = micros();

    Serial.println("Motor started (continuous rotation)");
}

/**
 * 停止电机
 */
void stepper_motor_stop() {
    motor_state.is_running = false;
    motor_state.remaining_steps = 0;

    // 关闭所有引脚
    PORTE &= ~((1 << PE0) | (1 << PE1) | (1 << PE2) | (1 << PE3));

    Serial.println("Motor stopped");
}

/**
 * 检查电机是否正在运行
 */
bool stepper_motor_is_running() {
    return motor_state.is_running;
}

/**
 * 更新电机状态 (需要在主循环中调用)
 */
void stepper_motor_update() {
    if (!motor_state.is_running) return;

    unsigned long current_time = micros();
    unsigned long delay_time = speed_delays[motor_state.speed];

    // 检查是否到了下一步的时间
    if (current_time - motor_state.last_step_time >= delay_time) {
        stepper_motor_step();
        motor_state.last_step_time = current_time;

        // 如果不是连续转动，检查是否完成目标步数
        if (motor_state.remaining_steps > 0) {
            motor_state.remaining_steps--;
            if (motor_state.remaining_steps == 0) {
                stepper_motor_stop();
            }
        }
    }
}

/**
 * 执行一步
 */
void stepper_motor_step() {
    // 根据方向更新步数
    if (motor_state.direction == CLOCKWISE) {
        motor_state.current_step++;
        if (motor_state.current_step >= STEP_SEQUENCE_LENGTH) {
            motor_state.current_step = 0;
        }
    } else {
        motor_state.current_step--;
        if (motor_state.current_step < 0) {
            motor_state.current_step = STEP_SEQUENCE_LENGTH - 1;
        }
    }

    // 设置引脚状态
    stepper_motor_set_pins(step_sequence[motor_state.current_step]);
}

/**
 * 设置引脚状态
 */
void stepper_motor_set_pins(uint8_t step_pattern) {
    // 清除所有引脚
    PORTE &= ~((1 << PE0) | (1 << PE1) | (1 << PE2) | (1 << PE3));

    // 根据步进模式设置引脚
    if (step_pattern & 0x01) PORTE |= (1 << PE0);  // INT1
    if (step_pattern & 0x02) PORTE |= (1 << PE1);  // INT2
    if (step_pattern & 0x04) PORTE |= (1 << PE2);  // INT3
    if (step_pattern & 0x08) PORTE |= (1 << PE3);  // INT4
}
