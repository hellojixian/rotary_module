#include "scan_mode.h"

// 3D扫描模式状态
static scan_mode_state_t scan_state;

// 时间常量
#define COUNTDOWN_DURATION_MS       3000
#define DISPLAY_UPDATE_INTERVAL_MS  500
#define STEPS_PER_REVOLUTION        200  // 步进电机每圈步数

/**
 * 初始化3D扫描模式
 */
void scan_mode_init(void) {
    scan_state.current_state = SCAN_STATE_IDLE;
    scan_state.state_enter_time = 0;
    scan_state.last_update_time = 0;
    scan_state.countdown_seconds = 0;
    scan_state.last_beep_time = 0;
    scan_state.start_time = 0;
    scan_state.total_runtime = 0;
    scan_state.total_steps = 0;
    scan_state.total_turns = 0.0;
    scan_state.last_display_update = 0;


}

/**
 * 启动3D扫描模式
 */
void scan_mode_start(void) {
    // 重置统计数据
    scan_state.total_steps = 0;
    scan_state.total_turns = 0.0;
    scan_state.start_time = 0;
    scan_state.total_runtime = 0;

    // 开始倒计时
    scan_mode_start_countdown();
}

/**
 * 停止3D扫描模式
 */
void scan_mode_stop(void) {
    // 停止电机
    stepper_motor_stop();

    // 计算总运行时间
    if (scan_state.start_time > 0) {
        scan_state.total_runtime = millis() - scan_state.start_time;
    }

    // 设置停止状态
    scan_state.current_state = SCAN_STATE_STOPPED;

    buzzer_tone(1000, 300);
}

/**
 * 更新3D扫描模式
 */
void scan_mode_update(void) {
    if (scan_state.current_state == SCAN_STATE_IDLE) {
        return;
    }

    unsigned long current_time = millis();
    scan_state.last_update_time = current_time;

    switch (scan_state.current_state) {
        case SCAN_STATE_COUNTDOWN:
            scan_mode_handle_countdown();
            break;
        case SCAN_STATE_RUNNING:
            scan_mode_handle_running();
            break;
        case SCAN_STATE_STOPPED:
            scan_state.current_state = SCAN_STATE_IDLE;
            break;
        default:
            break;
    }

    // 更新显示
    scan_mode_update_display();
}

/**
 * 检查3D扫描模式是否运行中
 */
bool scan_mode_is_running(void) {
    return (scan_state.current_state == SCAN_STATE_RUNNING ||
            scan_state.current_state == SCAN_STATE_COUNTDOWN);
}

/**
 * 获取3D扫描模式状态
 */
scan_state_t scan_mode_get_state(void) {
    return scan_state.current_state;
}

/**
 * 处理倒计时状态
 */
void scan_mode_handle_countdown(void) {
    unsigned long current_time = millis();
    unsigned long elapsed = current_time - scan_state.state_enter_time;

    // 计算剩余秒数
    uint8_t remaining_seconds = 3 - (elapsed / 1000);

    // 每秒beep一次
    if (remaining_seconds != scan_state.countdown_seconds) {
        scan_state.countdown_seconds = remaining_seconds;
        if (remaining_seconds > 0) {
            buzzer_tone(1500, 200);
            scan_state.last_beep_time = current_time;
        } else {
            buzzer_tone(2000, 200);
        }
    }

    // 倒计时结束，开始扫描
    if (elapsed >= COUNTDOWN_DURATION_MS) {
        scan_mode_start_scanning();
    }
}

/**
 * 处理运行状态
 */
void scan_mode_handle_running(void) {
    // 更新统计数据
    scan_mode_update_statistics();

    // 检查电机是否还在运行
    if (!stepper_motor_is_running()) {
        // 如果电机停止了，重新启动连续旋转
        stepper_motor_start();
    }
}

/**
 * 开始倒计时
 */
void scan_mode_start_countdown(void) {
    scan_state.current_state = SCAN_STATE_COUNTDOWN;
    scan_state.state_enter_time = millis();
    scan_state.countdown_seconds = 3;
}

/**
 * 开始扫描
 */
void scan_mode_start_scanning(void) {
    scan_state.current_state = SCAN_STATE_RUNNING;
    scan_state.state_enter_time = millis();
    scan_state.start_time = millis();

    // 设置电机参数
    uint8_t motor_speed = config_get_motor_speed();
    uint8_t motor_direction = config_get_motor_direction();

    // 根据配置设置电机速度
    if (motor_speed <= 4) {
        stepper_motor_set_speed(SPEED_HIGH);
    } else {
        stepper_motor_set_speed(SPEED_LOW);
    }

    // 设置电机方向
    stepper_motor_set_direction(motor_direction == MOTOR_DIRECTION_CW ? CLOCKWISE : COUNTER_CLOCKWISE);

    // 启动连续旋转
    stepper_motor_start();
}

/**
 * 更新统计数据
 */
void scan_mode_update_statistics(void) {
    // 这里需要从步进电机模块获取已移动的步数
    // 由于当前步进电机模块可能没有提供这个接口，我们使用时间估算

    if (scan_state.start_time > 0) {
        unsigned long elapsed_ms = millis() - scan_state.start_time;

        // 根据电机速度估算步数
        uint8_t motor_speed = config_get_motor_speed();
        float steps_per_second = 1000.0 / motor_speed;  // 每秒步数

        scan_state.total_steps = (uint32_t)(elapsed_ms * steps_per_second / 1000.0);
        scan_state.total_turns = (float)scan_state.total_steps / STEPS_PER_REVOLUTION;
    }
}

/**
 * 更新显示
 */
void scan_mode_update_display(void) {
    unsigned long current_time = millis();

    // 限制显示更新频率
    if (current_time - scan_state.last_display_update < DISPLAY_UPDATE_INTERVAL_MS) {
        return;
    }
    scan_state.last_display_update = current_time;

    switch (scan_state.current_state) {
        case SCAN_STATE_COUNTDOWN:
            ui_draw_countdown(scan_state.countdown_seconds);
            break;
        case SCAN_STATE_RUNNING:
            {
                unsigned long elapsed_seconds = scan_mode_get_elapsed_seconds();
                ui_draw_scan_running(scan_state.total_turns, elapsed_seconds);
            }
            break;
        default:
            break;
    }
}

/**
 * 计算旋转圈数
 */
float scan_mode_calculate_turns(void) {
    return scan_state.total_turns;
}

/**
 * 获取已运行秒数
 */
unsigned long scan_mode_get_elapsed_seconds(void) {
    if (scan_state.start_time == 0) {
        return 0;
    }
    return (millis() - scan_state.start_time) / 1000;
}
