#include "menu_system.h"
#include "buzzer.h"
#include "photo_mode.h"
#include "scan_mode.h"

// 菜单系统状态
static menu_system_state_t menu_state;

/**
 * 初始化菜单系统
 */
void menu_init(void) {
    menu_state.current_state = MENU_STATE_STANDBY;
    menu_state.previous_state = MENU_STATE_STANDBY;
    menu_state.state_enter_time = millis();
    menu_state.last_update_time = 0;
    menu_state.state_changed = true;
}

/**
 * 菜单系统主更新函数
 */
void menu_update(void) {
    unsigned long current_time = millis();

    // 检查相机状态变化
    menu_check_camera_status_change();

    // 处理按键事件
    menu_handle_key_events();

    // 处理当前状态
    switch (menu_state.current_state) {
        case MENU_STATE_STANDBY:
            menu_handle_standby_state();
            break;
        case MENU_STATE_CAMERA_MODE:
            menu_handle_camera_mode_state();
            break;
        case MENU_STATE_SCAN_MODE:
            menu_handle_scan_mode_state();
            break;
        case MENU_STATE_CONFIG:
            menu_handle_config_state();
            break;
        case MENU_STATE_CONFIG_EDIT:
            menu_handle_config_edit_state();
            break;
        case MENU_STATE_PHOTO_RUNNING:
            menu_handle_photo_running_state();
            break;
        case MENU_STATE_SCAN_RUNNING:
            menu_handle_scan_running_state();
            break;
        case MENU_STATE_COUNTDOWN:
            menu_handle_countdown_state();
            break;
    }

    // 更新显示
    menu_update_display();

    menu_state.last_update_time = current_time;
}

/**
 * 获取当前菜单状态
 */
menu_state_t menu_get_state(void) {
    return menu_state.current_state;
}

/**
 * 设置菜单状态
 */
void menu_set_state(menu_state_t new_state) {
    if (new_state != menu_state.current_state) {
        menu_state.previous_state = menu_state.current_state;
        menu_state.current_state = new_state;
        menu_state.state_enter_time = millis();
        menu_state.state_changed = true;

        // 强制更新UI
        ui_force_update();
    }
}

/**
 * 处理按键事件
 */
void menu_handle_key_events(void) {
    for (int i = 0; i < KEY_NUM_COUNT; i++) {
        key_event_t event = keys_get_event((key_num_t)i);
        if (event != KEY_EVENT_NONE) {
            menu_handle_key_press((key_num_t)i, event);
        }
    }
}

/**
 * 处理按键按下事件
 */
void menu_handle_key_press(key_num_t key, key_event_t event) {
    switch (key) {
        case KEY_NUM_CANCEL:
            menu_handle_cancel_key(event);
            break;
        case KEY_NUM_PREV:
            menu_handle_prev_key(event);
            break;
        case KEY_NUM_NEXT:
            menu_handle_next_key(event);
            break;
        case KEY_NUM_OK:
            menu_handle_ok_key(event);
            break;
    }
}

/**
 * 处理CANCEL键
 */
void menu_handle_cancel_key(key_event_t event) {
    switch (menu_state.current_state) {
        case MENU_STATE_STANDBY:
        case MENU_STATE_CAMERA_MODE:
        case MENU_STATE_SCAN_MODE:
            if (event == KEY_EVENT_LONG_PRESS) {
                menu_enter_config();
                buzzer_tone(1500, 200);
            }
            break;

        case MENU_STATE_CONFIG:
            if (event == KEY_EVENT_SHORT_PRESS) {
                menu_enter_standby();
                buzzer_tone(1200, 200);
            }
            break;

        case MENU_STATE_CONFIG_EDIT:
            if (event == KEY_EVENT_SHORT_PRESS) {
                menu_exit_config_edit();
                buzzer_tone(1200, 200);
            }
            break;

        case MENU_STATE_PHOTO_RUNNING:
        case MENU_STATE_SCAN_RUNNING:
            if (event == KEY_EVENT_SHORT_PRESS) {
                menu_stop_running_mode();
                buzzer_tone(1000, 300);
            }
            break;
    }
}

/**
 * 处理PREV键
 */
void menu_handle_prev_key(key_event_t event) {
    if (event != KEY_EVENT_SHORT_PRESS) return;

    switch (menu_state.current_state) {
        case MENU_STATE_CONFIG:
            ui_config_prev_item();
            buzzer_tone(1600, 100);
            break;

        case MENU_STATE_CONFIG_EDIT:
            ui_config_decrease_value();
            buzzer_tone(1400, 100);
            break;
    }
}

/**
 * 处理NEXT键
 */
void menu_handle_next_key(key_event_t event) {
    if (event != KEY_EVENT_SHORT_PRESS) return;

    switch (menu_state.current_state) {
        case MENU_STATE_CONFIG:
            ui_config_next_item();
            buzzer_tone(1700, 100);
            break;

        case MENU_STATE_CONFIG_EDIT:
            ui_config_increase_value();
            buzzer_tone(1800, 100);
            break;
    }
}

/**
 * 处理OK键
 */
void menu_handle_ok_key(key_event_t event) {
    if (event != KEY_EVENT_SHORT_PRESS) return;

    switch (menu_state.current_state) {
        case MENU_STATE_CAMERA_MODE:
            menu_start_photo_mode();
            break;

        case MENU_STATE_SCAN_MODE:
            menu_start_scan_mode();
            break;

        case MENU_STATE_CONFIG:
            menu_enter_config_edit();
            buzzer_tone(1800, 200);
            break;

        case MENU_STATE_CONFIG_EDIT:
            menu_exit_config_edit();
            config_save_to_eeprom();
            buzzer_tone(2000, 200);
            break;

        case MENU_STATE_PHOTO_RUNNING:
        case MENU_STATE_SCAN_RUNNING:
            menu_stop_running_mode();
            buzzer_tone(1000, 300);
            break;
    }
}

/**
 * 处理待机状态
 */
void menu_handle_standby_state(void) {
    // 根据相机状态自动切换到相应模式
    camera_status_t camera_status = camera_get_status();

    if (camera_status == CAMERA_FULLY_CONNECTED) {
        menu_enter_camera_mode();
    } else {
        menu_enter_scan_mode();
    }
}

/**
 * 处理相机模式状态
 */
void menu_handle_camera_mode_state(void) {
    // 检查相机是否断开连接
    if (camera_get_status() != CAMERA_FULLY_CONNECTED) {
        menu_enter_scan_mode();
    }
}

/**
 * 处理3D扫描模式状态
 */
void menu_handle_scan_mode_state(void) {
    // 检查相机是否连接
    if (camera_get_status() == CAMERA_FULLY_CONNECTED) {
        menu_enter_camera_mode();
    }
}

/**
 * 处理配置状态
 */
void menu_handle_config_state(void) {
    // 配置状态的处理在按键事件中完成
}

/**
 * 处理配置编辑状态
 */
void menu_handle_config_edit_state(void) {
    // 配置编辑状态的处理在按键事件中完成
}

/**
 * 处理拍照运行状态
 */
void menu_handle_photo_running_state(void) {
    // 检查拍照模式是否完成
    if (!photo_mode_is_running()) {
        menu_enter_standby();
    }
}

/**
 * 处理3D扫描运行状态
 */
void menu_handle_scan_running_state(void) {
    // 检查3D扫描模式是否完成
    if (!scan_mode_is_running()) {
        menu_enter_standby();
    }
}

/**
 * 处理倒计时状态
 */
void menu_handle_countdown_state(void) {
    // 倒计时状态的处理在相应的模式模块中完成
}

/**
 * 进入待机状态
 */
void menu_enter_standby(void) {
    menu_set_state(MENU_STATE_STANDBY);
}

/**
 * 进入相机模式
 */
void menu_enter_camera_mode(void) {
    menu_set_state(MENU_STATE_CAMERA_MODE);
}

/**
 * 进入3D扫描模式
 */
void menu_enter_scan_mode(void) {
    menu_set_state(MENU_STATE_SCAN_MODE);
}

/**
 * 进入配置模式
 */
void menu_enter_config(void) {
    menu_set_state(MENU_STATE_CONFIG);
}

/**
 * 进入配置编辑模式
 */
void menu_enter_config_edit(void) {
    menu_set_state(MENU_STATE_CONFIG_EDIT);
    ui_config_enter_edit();
}

/**
 * 退出配置编辑模式
 */
void menu_exit_config_edit(void) {
    ui_config_exit_edit();
    menu_set_state(MENU_STATE_CONFIG);
}

/**
 * 启动拍照模式
 */
void menu_start_photo_mode(void) {
    menu_set_state(MENU_STATE_PHOTO_RUNNING);
    photo_mode_start();
}

/**
 * 启动3D扫描模式
 */
void menu_start_scan_mode(void) {
    menu_set_state(MENU_STATE_SCAN_RUNNING);
    scan_mode_start();
}

/**
 * 停止运行模式
 */
void menu_stop_running_mode(void) {
    // 停止相应的模式
    if (menu_state.current_state == MENU_STATE_PHOTO_RUNNING) {
        photo_mode_stop();
    } else if (menu_state.current_state == MENU_STATE_SCAN_RUNNING) {
        scan_mode_stop();
    }

    menu_enter_standby();
}

/**
 * 更新显示
 */
void menu_update_display(void) {
    // 清屏
    display.clearDisplay();

    // 根据当前状态决定是否显示状态栏
    bool show_status_bar = true;
    if (menu_state.current_state == MENU_STATE_CONFIG ||
        menu_state.current_state == MENU_STATE_CONFIG_EDIT) {
        show_status_bar = false;
    }

    // 绘制状态栏（如果需要）
    if (show_status_bar) {
        ui_draw_status_bar();
        ui_draw_separator();
    }

    // 根据当前状态绘制主内容
    switch (menu_state.current_state) {
        case MENU_STATE_STANDBY:
            ui_draw_standby();
            break;
        case MENU_STATE_CAMERA_MODE:
            ui_draw_camera_mode();
            break;
        case MENU_STATE_SCAN_MODE:
            ui_draw_scan_mode();
            break;
        case MENU_STATE_CONFIG:
            ui_draw_config_menu_fullscreen();
            break;
        case MENU_STATE_CONFIG_EDIT:
            ui_draw_config_edit_fullscreen();
            break;
        case MENU_STATE_PHOTO_RUNNING:
            // 拍照模式的显示由photo_mode模块处理
            break;
        case MENU_STATE_SCAN_RUNNING:
            // 3D扫描模式的显示由scan_mode模块处理
            break;
        case MENU_STATE_COUNTDOWN:
            // 倒计时的显示由相应模块处理
            break;
    }

    display.display();
}

/**
 * 检查相机状态变化
 */
void menu_check_camera_status_change(void) {
    static camera_status_t last_camera_status = CAMERA_DISCONNECTED;
    camera_status_t current_status = camera_get_status();

    if (current_status != last_camera_status) {
        last_camera_status = current_status;

        // 如果在待机、相机模式或扫描模式，根据相机状态切换
        if (menu_state.current_state == MENU_STATE_STANDBY ||
            menu_state.current_state == MENU_STATE_CAMERA_MODE ||
            menu_state.current_state == MENU_STATE_SCAN_MODE) {

            if (current_status == CAMERA_FULLY_CONNECTED) {
                menu_enter_camera_mode();
            } else {
                menu_enter_scan_mode();
            }
        }
    }
}


