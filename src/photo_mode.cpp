#include "photo_mode.h"

// 拍照模式状态
static photo_mode_state_t photo_state;

// 时间常量
#define COUNTDOWN_DURATION_MS       3000
#define FOCUS_DURATION_MS           500
#define SHUTTER_DURATION_MS         200
#define ROTATION_SETTLE_TIME_MS     500

/**
 * 初始化拍照模式
 */
void photo_mode_init(void) {
    photo_state.current_state = PHOTO_STATE_IDLE;
    photo_state.state_enter_time = 0;
    photo_state.last_update_time = 0;
    photo_state.countdown_seconds = 0;
    photo_state.last_beep_time = 0;
    photo_state.total_photos = 0;
    photo_state.current_photo = 0;
    photo_state.target_angle = 0;
    photo_state.current_angle = 0;
    photo_state.angle_per_photo = 0;
    photo_state.steps_per_photo = 0;
    photo_state.total_steps_moved = 0;
    photo_state.focus_start_time = 0;
    photo_state.shutter_start_time = 0;
    photo_state.focus_triggered = false;
    photo_state.shutter_triggered = false;


}

/**
 * 启动拍照模式
 */
void photo_mode_start(void) {
    if (camera_get_status() != CAMERA_FULLY_CONNECTED) {
        buzzer_tone(1000, 500);
        return;
    }

    // 计算拍照参数
    photo_mode_calculate_parameters();

    // 开始倒计时
    photo_mode_start_countdown();
}

/**
 * 停止拍照模式
 */
void photo_mode_stop(void) {
    // 停止电机
    stepper_motor_stop();

    // 释放相机触发
    camera_release_triggers();

    // 重置状态
    photo_state.current_state = PHOTO_STATE_STOPPED;

    buzzer_tone(1000, 300);
}

/**
 * 更新拍照模式
 */
void photo_mode_update(void) {
    if (photo_state.current_state == PHOTO_STATE_IDLE) {
        return;
    }

    unsigned long current_time = millis();
    photo_state.last_update_time = current_time;

    switch (photo_state.current_state) {
        case PHOTO_STATE_COUNTDOWN:
            photo_mode_handle_countdown();
            break;
        case PHOTO_STATE_FOCUS:
            photo_mode_handle_focus();
            break;
        case PHOTO_STATE_FIRST_SHOT:
            photo_mode_handle_first_shot();
            break;
        case PHOTO_STATE_ROTATING:
            photo_mode_handle_rotating();
            break;
        case PHOTO_STATE_SHOOTING:
            photo_mode_handle_shooting();
            break;
        case PHOTO_STATE_COMPLETE:
            photo_mode_handle_complete();
            break;
        case PHOTO_STATE_STOPPED:
            photo_state.current_state = PHOTO_STATE_IDLE;
            break;
        default:
            break;
    }

    // 更新显示
    photo_mode_update_display();
}

/**
 * 检查拍照模式是否运行中
 */
bool photo_mode_is_running(void) {
    return (photo_state.current_state != PHOTO_STATE_IDLE &&
            photo_state.current_state != PHOTO_STATE_COMPLETE &&
            photo_state.current_state != PHOTO_STATE_STOPPED);
}

/**
 * 获取拍照模式状态
 */
photo_state_t photo_mode_get_state(void) {
    return photo_state.current_state;
}

/**
 * 处理倒计时状态
 */
void photo_mode_handle_countdown(void) {
    unsigned long current_time = millis();
    unsigned long elapsed = current_time - photo_state.state_enter_time;

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
    if (current_display_seconds != photo_state.countdown_seconds) {
        photo_state.countdown_seconds = current_display_seconds;

        // 同时触发蜂鸣（除了倒计时结束）
        if (current_display_seconds > 0) {
            buzzer_tone(1500, 200);
            photo_state.last_beep_time = current_time;
        }
    }

    // 倒计时结束，开始对焦
    if (elapsed >= COUNTDOWN_DURATION_MS) {
        buzzer_tone(2000, 200);  // 结束音
        photo_mode_trigger_focus();
    }
}

/**
 * 处理对焦状态
 */
void photo_mode_handle_focus(void) {
    unsigned long current_time = millis();
    unsigned long elapsed = current_time - photo_state.state_enter_time;

    // 对焦完成，拍摄第一张照片
    if (elapsed >= FOCUS_DURATION_MS) {
        photo_state.current_state = PHOTO_STATE_FIRST_SHOT;
        photo_state.state_enter_time = current_time;
        photo_state.current_photo = 1;
        photo_mode_trigger_shutter();
    }
}

/**
 * 处理第一张照片拍摄
 */
void photo_mode_handle_first_shot(void) {
    unsigned long current_time = millis();
    unsigned long elapsed = current_time - photo_state.state_enter_time;

    // 快门完成
    if (elapsed >= SHUTTER_DURATION_MS) {
        camera_release_triggers();

        // 如果只需要拍一张照片，直接完成
        if (photo_state.total_photos == 1) {
            photo_mode_finish_session();
        } else {
            // 开始旋转到下一个位置
            photo_mode_start_rotation();
        }
    }
}

/**
 * 处理旋转状态
 */
void photo_mode_handle_rotating(void) {
    // 检查电机是否完成旋转
    if (!stepper_motor_is_running()) {
        unsigned long current_time = millis();
        unsigned long elapsed = current_time - photo_state.state_enter_time;

        // 等待电机稳定
        if (elapsed >= ROTATION_SETTLE_TIME_MS) {
            // 更新当前角度
            photo_state.current_angle += photo_state.angle_per_photo;
            photo_state.current_photo++;

            // 检查是否完成所有拍摄
            if (photo_state.current_photo > photo_state.total_photos) {
                photo_mode_finish_session();
            } else {
                // 拍摄下一张照片
                photo_state.current_state = PHOTO_STATE_SHOOTING;
                photo_state.state_enter_time = current_time;
                photo_mode_trigger_shutter();
            }
        }
    }
}

/**
 * 处理拍摄状态
 */
void photo_mode_handle_shooting(void) {
    unsigned long current_time = millis();
    unsigned long elapsed = current_time - photo_state.state_enter_time;

    // 快门完成
    if (elapsed >= SHUTTER_DURATION_MS) {
        camera_release_triggers();

        // 检查是否完成所有拍摄
        if (photo_state.current_photo >= photo_state.total_photos) {
            photo_mode_finish_session();
        } else {
            // 继续旋转到下一个位置
            photo_mode_start_rotation();
        }
    }
}

/**
 * 处理完成状态
 */
void photo_mode_handle_complete(void) {
    // 播放完成提示音
    static bool completion_sound_played = false;
    if (!completion_sound_played) {
        buzzer_tone(2000, 200);
        delay(100);
        buzzer_tone(2500, 200);
        completion_sound_played = true;

        // 2秒后返回菜单
        photo_state.state_enter_time = millis();
    }

    // 2秒后自动返回空闲状态
    if (millis() - photo_state.state_enter_time >= 2000) {
        photo_state.current_state = PHOTO_STATE_IDLE;
    }
}

/**
 * 计算拍照参数
 */
void photo_mode_calculate_parameters(void) {
    uint16_t rotation_angle = config_get_rotation_angle();
    uint8_t photo_interval = config_get_photo_interval();

    // 计算总照片数
    photo_state.total_photos = (rotation_angle / photo_interval) + 1;
    photo_state.angle_per_photo = photo_interval;
    photo_state.target_angle = rotation_angle;
    photo_state.current_angle = 0;
    photo_state.current_photo = 0;

    // 计算每张照片需要的步数
    photo_state.steps_per_photo = photo_mode_angle_to_steps(photo_interval);
    photo_state.total_steps_moved = 0;
}

/**
 * 开始倒计时
 */
void photo_mode_start_countdown(void) {
    photo_state.current_state = PHOTO_STATE_COUNTDOWN;
    photo_state.state_enter_time = millis();
    photo_state.countdown_seconds = 3;
}

/**
 * 触发对焦
 */
void photo_mode_trigger_focus(void) {
    photo_state.current_state = PHOTO_STATE_FOCUS;
    photo_state.state_enter_time = millis();
    camera_trigger_focus();
}

/**
 * 触发快门
 */
void photo_mode_trigger_shutter(void) {
    camera_trigger_shutter();
}

/**
 * 开始旋转
 */
void photo_mode_start_rotation(void) {
    photo_state.current_state = PHOTO_STATE_ROTATING;
    photo_state.state_enter_time = millis();

    // 设置电机参数
    stepper_motor_set_direction(config_get_motor_direction() == MOTOR_DIRECTION_CW ? CLOCKWISE : COUNTER_CLOCKWISE);
    stepper_motor_set_custom_speed(config_get_motor_speed());

    // 开始旋转指定步数
    stepper_motor_rotate_steps(photo_state.steps_per_photo);
}

/**
 * 完成拍摄会话
 */
void photo_mode_finish_session(void) {
    photo_state.current_state = PHOTO_STATE_COMPLETE;
    photo_state.state_enter_time = millis();
}

/**
 * 更新显示
 */
void photo_mode_update_display(void) {
    // 清屏并绘制状态栏
    display.clearDisplay();
    ui_draw_status_bar();
    ui_draw_separator();

    // 根据拍照状态绘制内容
    switch (photo_state.current_state) {
        case PHOTO_STATE_COUNTDOWN:
            ui_draw_countdown(photo_state.countdown_seconds);
            break;
        case PHOTO_STATE_FOCUS:
        case PHOTO_STATE_FIRST_SHOT:
        case PHOTO_STATE_ROTATING:
        case PHOTO_STATE_SHOOTING:
            ui_draw_photo_running(photo_state.current_photo, photo_state.total_photos);
            break;
        case PHOTO_STATE_COMPLETE:
            ui_center_text("Photo Complete!", 16);
            break;
        default:
            break;
    }

    // 显示到屏幕
    display.display();
}

/**
 * 角度转换为步数
 */
uint32_t photo_mode_angle_to_steps(uint16_t angle) {
    // 假设步进电机每圈200步，减速比1:1
    // 360度 = 200步
    return (uint32_t)angle * 200 / 360;
}

/**
 * 步数转换为角度
 */
uint16_t photo_mode_steps_to_angle(uint32_t steps) {
    return (uint16_t)(steps * 360 / 200);
}
