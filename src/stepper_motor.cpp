#include <Arduino.h>
#include "stepper_motor.h"

// 步进电机全局状态
static stepper_motor_t motor_state;

// 速度延时配置 (微秒) - 优化为平滑运行和降低发热
static const unsigned long speed_delays[] = {
    7000,   // SPEED_LOW: 7ms延时 (500步/秒，平滑稳定，低发热)
    2000    // SPEED_HIGH: 2ms延时 (1250步/秒，快速但平滑)
};

// 自定义速度延时 (微秒)
static unsigned long custom_speed_delay = 4000;  // 默认4ms

// 步数计数器
static volatile uint32_t step_counter = 0;

// 28BYJ-48 半步序列 (8步，平滑但扭矩较小)
static const uint8_t step_sequence_half[STEP_SEQUENCE_LENGTH_HALF] = {
    0b0001,  // 0001
    0b0011,  // 0011
    0b0010,  // 0010
    0b0110,  // 0110
    0b0100,  // 0100
    0b1100,  // 1100
    0b1000,  // 1000
    0b1001   // 1001
};

// 28BYJ-48 全步序列 (4步，扭矩更大)
static const uint8_t step_sequence_full[STEP_SEQUENCE_LENGTH_FULL] = {
    0b0011,  // 线圈1+2同时通电
    0b0110,  // 线圈2+3同时通电
    0b1100,  // 线圈3+4同时通电
    0b1001   // 线圈4+1同时通电
};

// 高扭矩模式标志
static bool high_torque_mode = false;

/**
 * 初始化步进电机
 */
void stepper_motor_init() {
    // 启用PE0和PE2引脚 (根据用户提供的代码)
    MCUSR = 0xff;
    MCUSR = 0xff;

    // 设置PE0-PE3为输出模式
    DDRE |= (1 << PE0) | (1 << PE1) | (1 << PE2) | (1 << PE3);

    // 初始化所有引脚为低电平
    PORTE &= ~((1 << PE0) | (1 << PE1) | (1 << PE2) | (1 << PE3));

    // 初始化电机状态
    motor_state.current_step = 0;
    motor_state.direction = CLOCKWISE;
    motor_state.speed = SPEED_LOW;
    motor_state.step_mode = STEP_MODE_FULL;
    motor_state.is_running = false;
    motor_state.last_step_time = 0;
    motor_state.target_steps = 0;
    motor_state.remaining_steps = 0;

    // 禁用高扭矩模式以降低发热
    high_torque_mode = false;

    stepper_motor_stop();
    Serial.println(F("Stepper motor initialized (Low Heat Mode - Half Step)"));
}

/**
 * 设置电机速度
 */
void stepper_motor_set_speed(motor_speed_t speed) {
    motor_state.speed = speed;
}

/**
 * 设置自定义电机速度
 * @param delay_ms 步进间隔时间（毫秒）
 */
void stepper_motor_set_custom_speed(uint8_t delay_ms) {
    // 限制范围在2-100ms之间
    if (delay_ms < 2) delay_ms = 2;
    if (delay_ms > 100) delay_ms = 100;

    // 转换为微秒
    custom_speed_delay = (unsigned long)delay_ms * 1000;
}

/**
 * 设置电机转动方向
 */
void stepper_motor_set_direction(motor_direction_t direction) {
    motor_state.direction = direction;
    Serial.print(F("Motor direction set to: "));
    Serial.println(direction == CLOCKWISE ? F("CLOCKWISE") : F("COUNTER_CLOCKWISE"));
}

/**
 * 根据角度旋转电机
 * @param angle 角度 (度)，正数顺时针，负数逆时针
 */
void stepper_motor_rotate_angle(float angle) {
    // 根据步进模式计算需要的步数
    int steps_per_rev = (motor_state.step_mode == STEP_MODE_FULL) ?
                        STEPS_PER_REVOLUTION_FULL : STEPS_PER_REVOLUTION_HALF;
    int steps = (int)((angle / 360.0) * steps_per_rev);

    // 根据角度符号设置方向
    if (steps < 0) {
        motor_state.direction = COUNTER_CLOCKWISE;
        steps = -steps;
    } else {
        motor_state.direction = CLOCKWISE;
    }

    stepper_motor_rotate_steps(steps);

    Serial.print(F("Rotating "));
    Serial.print(angle);
    Serial.print(F(" degrees ("));
    Serial.print(steps);
    Serial.print(F(" steps, "));
    Serial.print(motor_state.step_mode == STEP_MODE_FULL ? F("FULL") :F( "HALF"));
    Serial.println(F(" step mode)"));
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

    Serial.print(F("Starting rotation: "));
    Serial.print(steps);
    Serial.println(F(" steps"));
}

/**
 * 启动电机连续转动
 */
void stepper_motor_start() {
    motor_state.is_running = true;
    motor_state.remaining_steps = -1; // -1表示连续转动
    motor_state.last_step_time = micros();
    Serial.println(F("Motor started (continuous rotation)"));
}

/**
 * 停止电机
 */
void stepper_motor_stop() {
    motor_state.is_running = false;
    motor_state.remaining_steps = 0;

    // 关闭所有引脚
    PORTE &= ~((1 << PE0) | (1 << PE1) | (1 << PE2) | (1 << PE3));

    Serial.println(F("Motor stopped"));
}

/**
 * 检查电机是否正在运行
 */
bool stepper_motor_is_running() {
    return motor_state.is_running;
}

/**
 * 获取当前步进模式
 */
step_mode_t stepper_motor_get_step_mode() {
    return motor_state.step_mode;
}

/**
 * 获取步数计数器
 */
uint32_t stepper_motor_get_step_count() {
    return step_counter;
}

/**
 * 重置步数计数器
 */
void stepper_motor_reset_step_count() {
    step_counter = 0;
}

/**
 * 更新电机状态 (需要在主循环中调用)
 */
void stepper_motor_update() {
    if (!motor_state.is_running) return;

    unsigned long current_time = micros();
    // 优先使用自定义速度，如果没有设置则使用预设速度
    unsigned long delay_time = (custom_speed_delay > 0) ? custom_speed_delay : speed_delays[motor_state.speed];
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
    // 获取当前步进模式的序列长度
    int sequence_length = (motor_state.step_mode == STEP_MODE_FULL) ?
                         STEP_SEQUENCE_LENGTH_FULL : STEP_SEQUENCE_LENGTH_HALF;

    // 根据方向更新步数
    if (motor_state.direction == CLOCKWISE) {
        motor_state.current_step++;
        if (motor_state.current_step >= sequence_length) {
            motor_state.current_step = 0;
        }
    } else {
        motor_state.current_step--;
        if (motor_state.current_step < 0) {
            motor_state.current_step = sequence_length - 1;
        }
    }

    // 根据步进模式选择序列并设置引脚状态
    if (motor_state.step_mode == STEP_MODE_FULL) {
        stepper_motor_set_pins(step_sequence_full[motor_state.current_step]);
    } else {
        stepper_motor_set_pins(step_sequence_half[motor_state.current_step]);
    }

    // 增加步数计数器
    step_counter++;
}

/**
 * 设置步进模式
 */
void stepper_motor_set_step_mode(step_mode_t mode) {
    motor_state.step_mode = mode;
    motor_state.current_step = 0;  // 重置步数位置

    Serial.print(F("Step mode set to: "));
    Serial.println(mode == STEP_MODE_FULL ? F("FULL (High Torque)") : F("HALF (Smooth)"));
}

/**
 * 启用高扭矩模式
 */
void stepper_motor_enable_high_torque() {
    stepper_motor_set_step_mode(STEP_MODE_FULL);
    high_torque_mode = true;

    Serial.println(F("High torque mode enabled"));
}

/**
 * 禁用高扭矩模式
 */
void stepper_motor_disable_high_torque() {
    stepper_motor_set_step_mode(STEP_MODE_HALF);
    high_torque_mode = false;

    Serial.println(F("High torque mode disabled"));
}

/**
 * 启用低发热模式
 */
void stepper_motor_enable_low_heat_mode() {
    // 切换到半步模式
    stepper_motor_set_step_mode(STEP_MODE_HALF);

    // 禁用高扭矩模式
    high_torque_mode = false;

    // 设置为低速以进一步降低发热
    stepper_motor_set_speed(SPEED_LOW);

    Serial.println(F("Low heat mode enabled (Half step, Low speed)"));
}

/**
 * 设置引脚状态
 */
void stepper_motor_set_pins(uint8_t step_pattern) {
    // 清除所有引脚 - 确保完全断电
    PORTE &= ~((1 << PE0) | (1 << PE1) | (1 << PE2) | (1 << PE3));

    // 根据步进模式设置引脚
    if (step_pattern & 0x01) PORTE |= (1 << PE0);  // INT1
    if (step_pattern & 0x02) PORTE |= (1 << PE1);  // INT2
    if (step_pattern & 0x04) PORTE |= (1 << PE2);  // INT3
    if (step_pattern & 0x08) PORTE |= (1 << PE3);  // INT4

    if (high_torque_mode) {
        delayMicroseconds(50);
    }
}
