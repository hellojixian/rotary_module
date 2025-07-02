#include "photo_mode.h"

// 拍照模式状态
static photo_mode_state_t photo_state;

// 时间常量
#define COUNTDOWN_DURATION_MS       3000
#define FOCUS_DURATION_MS           500
#define SHUTTER_DURATION_MS         200
#define ROTATION_SETTLE_TIME_MS     500
#define PHOTO_DISPLAY_UPDATE_INTERVAL_MS  50  // 拍照模式高频显示更新间隔

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
    photo_state.last_display_update = 0;


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
        case PHOTO_STATE_PRE_FIRST_SHOT:
            photo_mode_handle_pre_first_shot();
            break;
        case PHOTO_STATE_FIRST_SHOT:
            photo_mode_handle_first_shot();
            break;
        case PHOTO_STATE_POST_FIRST_SHOT:
            photo_mode_handle_post_first_shot();
            break;
        case PHOTO_STATE_ROTATING:
            photo_mode_handle_rotating();
            break;
        case PHOTO_STATE_PRE_SHOOTING:
            photo_mode_handle_pre_shooting();
            break;
        case PHOTO_STATE_SHOOTING:
            photo_mode_handle_shooting();
            break;
        case PHOTO_STATE_POST_SHOOTING:
            photo_mode_handle_post_shooting();
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

    // 添加调试信息显示对焦进度
    static unsigned long last_debug_time = 0;
    if (current_time - last_debug_time >= 500) {  // 每500ms输出一次
        last_debug_time = current_time;
    }

    // 对焦完成，进入第一张照片前停留状态
    if (elapsed >= CAMERA_FOCUS_TRIGGER_TIME) {
        // 释放对焦触发
        camera_release_triggers();

        photo_state.current_state = PHOTO_STATE_PRE_FIRST_SHOT;
        photo_state.state_enter_time = current_time;
        // current_photo保持为0，表示还没有完成任何照片
    }
}

/**
 * 处理第一张照片前停留状态
 */
void photo_mode_handle_pre_first_shot(void) {
    unsigned long current_time = millis();
    unsigned long elapsed = current_time - photo_state.state_enter_time;

    // 停留时间结束，开始拍摄第一张照片
    if (elapsed >= PHOTO_PRE_SHUTTER_SETTLE_TIME) {
        photo_state.current_state = PHOTO_STATE_FIRST_SHOT;
        photo_state.state_enter_time = current_time;
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

        // 进入第一张照片后停留状态
        photo_state.current_state = PHOTO_STATE_POST_FIRST_SHOT;
        photo_state.state_enter_time = current_time;
    }
}

/**
 * 处理第一张照片后停留状态
 */
void photo_mode_handle_post_first_shot(void) {
    unsigned long current_time = millis();
    unsigned long elapsed = current_time - photo_state.state_enter_time;

    // 停留时间结束
    if (elapsed >= PHOTO_POST_SHUTTER_SETTLE_TIME) {
        // 第一张照片已完成
        photo_state.current_photo = 1;

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
    // 实时更新当前角度显示（基于电机已完成的步数）
    uint32_t completed_steps = stepper_motor_get_current_rotation_steps();
    uint16_t current_rotation_angle = photo_mode_steps_to_angle(completed_steps);

    // 更新显示用的当前角度（基础角度 + 当前旋转进度）
    uint16_t base_angle = photo_state.current_photo * photo_state.angle_per_photo;
    photo_state.current_angle = base_angle + current_rotation_angle;

    // 检查电机是否完成旋转
    if (!stepper_motor_is_running()) {
        unsigned long current_time = millis();
        unsigned long elapsed = current_time - photo_state.state_enter_time;

        // 等待电机稳定
        if (elapsed >= ROTATION_SETTLE_TIME_MS) {
            // 确保角度更新到精确位置
            photo_state.current_angle = (photo_state.current_photo + 1) * photo_state.angle_per_photo;

            // 检查是否已经拍摄完所有照片
            // 最后一次旋转是为了复位，不需要拍摄
            if (photo_state.current_photo >= photo_state.total_photos) {
                photo_mode_finish_session();
            } else {
                // 进入拍摄前停留状态
                photo_state.current_state = PHOTO_STATE_PRE_SHOOTING;
                photo_state.state_enter_time = current_time;
            }
        }
    }
}

/**
 * 处理拍摄前停留状态
 */
void photo_mode_handle_pre_shooting(void) {
    unsigned long current_time = millis();
    unsigned long elapsed = current_time - photo_state.state_enter_time;

    // 停留时间结束，开始拍摄
    if (elapsed >= PHOTO_PRE_SHUTTER_SETTLE_TIME) {
        photo_state.current_state = PHOTO_STATE_SHOOTING;
        photo_state.state_enter_time = current_time;
        photo_mode_trigger_shutter();
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

        // 进入拍摄后停留状态
        photo_state.current_state = PHOTO_STATE_POST_SHOOTING;
        photo_state.state_enter_time = current_time;
    }
}

/**
 * 处理拍摄后停留状态
 */
void photo_mode_handle_post_shooting(void) {
    unsigned long current_time = millis();
    unsigned long elapsed = current_time - photo_state.state_enter_time;

    // 停留时间结束
    if (elapsed >= PHOTO_POST_SHUTTER_SETTLE_TIME) {
        // 当前照片已完成
        photo_state.current_photo++;

        // 检查是否已经拍摄完所有照片
        if (photo_state.current_photo >= photo_state.total_photos) {
            // 已经拍摄完所有照片，最后一次旋转是为了复位
            photo_mode_start_rotation();
        } else {
            // 继续旋转到下一个位置进行拍摄
            photo_mode_start_rotation();
        }
    }
}

/**
 * 处理完成状态
 */
void photo_mode_handle_complete(void) {
    unsigned long current_time = millis();
    unsigned long elapsed = current_time - photo_state.state_enter_time;

    // 使用状态变量来跟踪音效播放，避免静态变量问题
    static unsigned long first_beep_time = 0;
    static unsigned long second_beep_time = 0;
    static bool beeps_initialized = false;

    // 初始化音效时间（只在第一次进入完成状态时）
    if (!beeps_initialized) {
        first_beep_time = current_time;
        second_beep_time = current_time + 150;  // 300ms后播放第二个音
        beeps_initialized = true;
        buzzer_tone(1500, 100);  // 播放第一个音
    }

    // 播放第二个音效
    if (current_time >= second_beep_time && second_beep_time > 0) {
        buzzer_tone(2000, 150);
        second_beep_time = 0;  // 标记已播放，避免重复
    }

    // 2秒后自动返回空闲状态
    if (elapsed >= 2000) {
        photo_state.current_state = PHOTO_STATE_IDLE;
        // 重置音效状态，为下次使用做准备
        beeps_initialized = false;
        first_beep_time = 0;
        second_beep_time = 0;
    }
}

/**
 * 计算拍照参数
 */
void photo_mode_calculate_parameters(void) {
    uint16_t rotation_angle = config_get_rotation_angle();
    uint8_t photo_interval = config_get_photo_interval();

    // 计算总照片数：第一张在0度拍摄，然后每隔photo_interval拍摄一张
    // 例如：360度，间隔30度 -> 0°, 30°, 60°, 90°, 120°, 150°, 180°, 210°, 240°, 270°, 300°, 330° = 12张照片
    // 不在360度位置拍摄，因为那接近起始位置
    photo_state.total_photos = rotation_angle / photo_interval;
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
    unsigned long current_time = millis();

    // 限制显示更新频率为50ms（20fps），提供流畅的进度条更新
    if (current_time - photo_state.last_display_update < PHOTO_DISPLAY_UPDATE_INTERVAL_MS) {
        return;
    }
    photo_state.last_display_update = current_time;

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
        case PHOTO_STATE_PRE_FIRST_SHOT:
        case PHOTO_STATE_FIRST_SHOT:
        case PHOTO_STATE_POST_FIRST_SHOT:
        case PHOTO_STATE_ROTATING:
        case PHOTO_STATE_PRE_SHOOTING:
        case PHOTO_STATE_SHOOTING:
        case PHOTO_STATE_POST_SHOOTING:
            // 直接使用 current_photo，它现在始终表示已完成的照片数
            ui_draw_photo_running(photo_state.current_photo, photo_state.total_photos,
                                 photo_state.target_angle, photo_state.angle_per_photo);
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
    // 根据当前步进模式获取正确的每圈步数
    step_mode_t step_mode = stepper_motor_get_step_mode();
    uint16_t steps_per_revolution = (step_mode == STEP_MODE_FULL) ?
                                   STEPS_PER_REVOLUTION_FULL : STEPS_PER_REVOLUTION_HALF;

    // 计算角度对应的步数
    return (uint32_t)angle * steps_per_revolution / 360;
}

/**
 * 步数转换为角度
 */
uint16_t photo_mode_steps_to_angle(uint32_t steps) {
    // 根据当前步进模式获取正确的每圈步数
    step_mode_t step_mode = stepper_motor_get_step_mode();
    uint16_t steps_per_revolution = (step_mode == STEP_MODE_FULL) ?
                                   STEPS_PER_REVOLUTION_FULL : STEPS_PER_REVOLUTION_HALF;

    // 计算步数对应的角度
    return (uint16_t)(steps * 360 / steps_per_revolution);
}
