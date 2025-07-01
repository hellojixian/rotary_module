#include "scan_mode.h"
#include "stepper_motor.h"

// 3D扫描模式状态
static scan_mode_state_t scan_state;

// 时间常量
#define COUNTDOWN_DURATION_MS       3000
#define DISPLAY_UPDATE_INTERVAL_MS  500

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

    // 计算当前显示的秒数（从3开始倒数）
    uint8_t current_display_seconds;
    if (elapsed < 1000) {
        current_display_seconds = 3;
    } else if (elapsed < 2000) {
        current_display_seconds = 2;
    } else if (elapsed < 3000) {
        current_display_seconds = 1;
    } else {
        current_display_seconds = 0;
    }

    // 检查是否需要更新显示和蜂鸣
    if (current_display_seconds != scan_state.countdown_seconds) {
        scan_state.countdown_seconds = current_display_seconds;

        // 同时触发蜂鸣（除了倒计时结束）
        if (current_display_seconds > 0) {
            buzzer_tone(1500, 200);
            scan_state.last_beep_time = current_time;
        }
    }

    // 倒计时结束，开始扫描
    if (elapsed >= COUNTDOWN_DURATION_MS) {
        buzzer_tone(2000, 200);  // 结束音
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

    // 直接使用用户配置的毫秒数
    stepper_motor_set_custom_speed(motor_speed);

    // 设置电机方向
    stepper_motor_set_direction(motor_direction == MOTOR_DIRECTION_CW ? CLOCKWISE : COUNTER_CLOCKWISE);

    // 重置步数计数器
    stepper_motor_reset_step_count();

    // 启动连续旋转
    stepper_motor_start();
}

/**
 * 更新统计数据
 */
void scan_mode_update_statistics(void) {
    // 直接从步进电机模块读取精确的步数
    scan_state.total_steps = stepper_motor_get_step_count();

    // 根据当前步进模式计算圈数
    step_mode_t step_mode = stepper_motor_get_step_mode();
    uint16_t steps_per_revolution = (step_mode == STEP_MODE_FULL) ?
                                   STEPS_PER_REVOLUTION_FULL : STEPS_PER_REVOLUTION_HALF;

    scan_state.total_turns = (float)scan_state.total_steps / steps_per_revolution;
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

    // 清屏并绘制状态栏
    display.clearDisplay();
    ui_draw_status_bar();
    ui_draw_separator();

    // 根据扫描状态绘制内容
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

    // 显示到屏幕
    display.display();
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
