#include "ui_display.h"

// 外部变量声明
extern float battery_voltage;

// UI状态变量
static ui_state_t ui_state;

/**
 * 初始化UI显示系统
 */
void ui_init(void) {
    ui_state.force_update = true;
    ui_state.last_update = 0;
    ui_state.current_config_item = CONFIG_ITEM_MOTOR_DIRECTION;
    ui_state.in_config_edit = false;


}

/**
 * 更新UI显示
 */
void ui_update(void) {
    unsigned long current_time = millis();

    // 每100ms更新一次，或强制更新
    if (ui_state.force_update || (current_time - ui_state.last_update >= 100)) {
        ui_state.last_update = current_time;
        ui_state.force_update = false;

        // 清屏
        display.clearDisplay();

        // 绘制状态栏
        ui_draw_status_bar();

        // 绘制分隔线
        ui_draw_separator();

        // 显示内容
        display.display();
    }
}

/**
 * 强制更新UI
 */
void ui_force_update(void) {
    ui_state.force_update = true;
}

/**
 * 绘制状态栏
 */
void ui_draw_status_bar(void) {
    // 绘制相机状态（左上角）
    ui_draw_camera_status();

    // 绘制电池状态（右上角）
    ui_draw_battery_status();
}

/**
 * 绘制相机状态
 */
void ui_draw_camera_status(void) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    camera_status_t status = camera_get_status();
    switch (status) {
        case CAMERA_DISCONNECTED:
            display.print(F("3D Scan Mode"));
            break;
        case CAMERA_CABLE_CONNECTED:
            display.print(F("Connecting..."));
            break;
        case CAMERA_FULLY_CONNECTED:
            display.print(F("Camera Mode"));
            break;
    }
}

/**
 * 绘制电池状态
 */
void ui_draw_battery_status(void) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // 计算电池电压显示位置（右对齐）
    display.setCursor(98, 0);  // 右上角位置
    display.print(battery_voltage, 2);
    display.print("V");
}

/**
 * 绘制分隔线
 */
void ui_draw_separator(void) {
    display.drawLine(0, UI_STATUS_BAR_HEIGHT, SCREEN_WIDTH - 1, UI_STATUS_BAR_HEIGHT, SSD1306_WHITE);
}

/**
 * 绘制待机界面
 */
void ui_draw_standby(void) {
    ui_center_text("Rotary Platform", UI_STATUS_BAR_HEIGHT + UI_SEPARATOR_HEIGHT + 8);
}

/**
 * 绘制相机模式界面
 */
void ui_draw_camera_mode(void) {
    ui_center_text("Press OK to Start", UI_STATUS_BAR_HEIGHT + UI_SEPARATOR_HEIGHT + 4);
    ui_center_text("Photo Mode", UI_STATUS_BAR_HEIGHT + UI_SEPARATOR_HEIGHT + 14);
}

/**
 * 绘制3D扫描模式界面
 */
void ui_draw_scan_mode(void) {
    ui_center_text("Press OK to Start", UI_STATUS_BAR_HEIGHT + UI_SEPARATOR_HEIGHT + 4);
    ui_center_text("3D Scan Mode", UI_STATUS_BAR_HEIGHT + UI_SEPARATOR_HEIGHT + 14);
}

/**
 * 绘制配置菜单
 */
void ui_draw_config_menu(void) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // 只显示当前选中的配置项，居中显示
    uint8_t y = UI_STATUS_BAR_HEIGHT + UI_SEPARATOR_HEIGHT + 8;

    // 显示配置项名称
    display.setCursor(0, y);
    display.print(F("> "));
    display.print(ui_get_config_item_name(ui_state.current_config_item));

    // 显示配置项值，换行显示
    display.setCursor(0, y + 10);
    display.print(F("  "));
    ui_print_config_item_value(ui_state.current_config_item);
}

/**
 * 绘制配置编辑界面
 */
void ui_draw_config_edit(void) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    uint8_t y = UI_STATUS_BAR_HEIGHT + UI_SEPARATOR_HEIGHT + 8;

    display.setCursor(0, y);
    display.print(F("Edit: "));
    display.print(ui_get_config_item_name(ui_state.current_config_item));

    display.setCursor(0, y + 10);
    display.print(F("Value: "));
    ui_print_config_item_value(ui_state.current_config_item);
}

/**
 * 绘制配置菜单（全屏模式）
 */
void ui_draw_config_menu_fullscreen(void) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // 使用全屏空间，从顶部开始
    uint8_t y = 4;

    // 显示标题
    // ui_center_text("Configuration", y);

    // 显示当前配置项，使用更大的字体和更多空间
    display.setTextSize(1);
    display.setCursor(0, y + 6);
    display.print(F("> "));
    display.print(ui_get_config_item_name(ui_state.current_config_item));

    // 显示配置项值，使用更大的字体
    display.setTextSize(1);
    display.setCursor(0, y + 18);
    display.print(F("  "));
    ui_print_config_item_value(ui_state.current_config_item);
}

/**
 * 绘制配置编辑界面（全屏模式）
 */
void ui_draw_config_edit_fullscreen(void) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // 使用全屏空间，从顶部开始
    uint8_t y = 2;

    // 显示标题
    ui_center_text("Edit Setting", y);

    // 显示配置项名称
    display.setCursor(0, y + 12);
    display.print(F("Item: "));
    display.print(ui_get_config_item_name(ui_state.current_config_item));

    // 显示当前值，使用更大的字体突出显示
    display.setTextSize(1);
    display.setCursor(0, y + 22);
    display.print(F("Value: "));
    ui_print_config_item_value(ui_state.current_config_item);
}

/**
 * 清除主内容区域
 */
void ui_clear_main_area(void) {
    display.fillRect(0, UI_STATUS_BAR_HEIGHT + UI_SEPARATOR_HEIGHT,
                    SCREEN_WIDTH, UI_MAIN_CONTENT_HEIGHT, SSD1306_BLACK);
}

/**
 * 绘制拍照运行界面
 */
void ui_draw_photo_running(uint8_t current_photo, uint8_t total_photos) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    uint8_t y = UI_STATUS_BAR_HEIGHT + UI_SEPARATOR_HEIGHT + 4;

    // 显示进度文本
    display.setCursor(0, y);
    display.print(F("Photo: "));
    display.print(current_photo);
    display.print(F("/"));
    display.print(total_photos);

    // 绘制进度条
    ui_draw_progress_bar(0, y + 10, UI_PROGRESS_BAR_WIDTH, UI_PROGRESS_BAR_HEIGHT,
                        current_photo, total_photos);
}

/**
 * 绘制3D扫描运行界面
 */
void ui_draw_scan_running(float turns, unsigned long elapsed_seconds) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    uint8_t y = UI_STATUS_BAR_HEIGHT + UI_SEPARATOR_HEIGHT + 4;

    // 显示旋转圈数
    display.setCursor(0, y);
    display.print(F("Scan: "));
    display.print(turns, 2);  // 保留2位小数
    display.print(F(" turns"));

    // 显示运行时间
    display.setCursor(0, y + 10);
    display.print(F("Time: "));

    // 直接计算并显示时间
    unsigned long minutes = elapsed_seconds / 60;
    unsigned long seconds = elapsed_seconds % 60;

    if (minutes < 10) display.print(F("0"));
    display.print(minutes);
    display.print(F(":"));
    if (seconds < 10) display.print(F("0"));
    display.print(seconds);
    Serial.println(seconds);
}

/**
 * 绘制倒计时界面
 */
void ui_draw_countdown(uint8_t seconds) {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    // 直接居中显示数字
    uint8_t x = (SCREEN_WIDTH - 12) / 2;  // 大字体数字大约12像素宽
    uint8_t y = UI_STATUS_BAR_HEIGHT + UI_SEPARATOR_HEIGHT + 10;

    display.setCursor(x, y);
    display.print(seconds);
}

/**
 * 绘制进度条
 */
void ui_draw_progress_bar(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                         uint8_t current, uint8_t total) {
    // 绘制边框
    display.drawRect(x, y, width, height, SSD1306_WHITE);

    // 计算填充宽度
    if (total > 0) {
        uint8_t fill_width = (width - 2) * current / total;
        if (fill_width > 0) {
            display.fillRect(x + 1, y + 1, fill_width, height - 2, SSD1306_WHITE);
        }
    }
}

/**
 * 配置菜单导航 - 下一项
 */
void ui_config_next_item(void) {
    ui_state.current_config_item = (config_item_t)((ui_state.current_config_item + 1) % CONFIG_ITEM_COUNT);
    ui_force_update();
}

/**
 * 配置菜单导航 - 上一项
 */
void ui_config_prev_item(void) {
    ui_state.current_config_item = (config_item_t)((ui_state.current_config_item + CONFIG_ITEM_COUNT - 1) % CONFIG_ITEM_COUNT);
    ui_force_update();
}

/**
 * 进入配置编辑模式
 */
void ui_config_enter_edit(void) {
    ui_state.in_config_edit = true;
    ui_force_update();
}

/**
 * 退出配置编辑模式
 */
void ui_config_exit_edit(void) {
    ui_state.in_config_edit = false;
    ui_force_update();
}

/**
 * 增加配置值
 */
void ui_config_increase_value(void) {
    switch (ui_state.current_config_item) {
        case CONFIG_ITEM_MOTOR_DIRECTION:
            config_set_motor_direction(MOTOR_DIRECTION_CCW);
            break;
        case CONFIG_ITEM_MOTOR_SPEED:
            if (config_get_motor_speed() < MOTOR_SPEED_MAX) {
                config_set_motor_speed(config_get_motor_speed() + 1);
            }
            break;
        case CONFIG_ITEM_ROTATION_ANGLE:
            {
                uint16_t current = config_get_rotation_angle();
                if (current == ROTATION_ANGLE_90) config_set_rotation_angle(ROTATION_ANGLE_180);
                else if (current == ROTATION_ANGLE_180) config_set_rotation_angle(ROTATION_ANGLE_360);
                else if (current == ROTATION_ANGLE_360) config_set_rotation_angle(ROTATION_ANGLE_540);
                else if (current == ROTATION_ANGLE_540) config_set_rotation_angle(ROTATION_ANGLE_720);
            }
            break;
        case CONFIG_ITEM_PHOTO_INTERVAL:
            {
                uint8_t current = config_get_photo_interval();
                if (current == PHOTO_INTERVAL_5) config_set_photo_interval(PHOTO_INTERVAL_10);
                else if (current == PHOTO_INTERVAL_10) config_set_photo_interval(PHOTO_INTERVAL_15);
                else if (current == PHOTO_INTERVAL_15) config_set_photo_interval(PHOTO_INTERVAL_30);
            }
            break;
    }
    ui_force_update();
}

/**
 * 减少配置值
 */
void ui_config_decrease_value(void) {
    switch (ui_state.current_config_item) {
        case CONFIG_ITEM_MOTOR_DIRECTION:
            config_set_motor_direction(MOTOR_DIRECTION_CW);
            break;
        case CONFIG_ITEM_MOTOR_SPEED:
            if (config_get_motor_speed() > MOTOR_SPEED_MIN) {
                config_set_motor_speed(config_get_motor_speed() - 1);
            }
            break;
        case CONFIG_ITEM_ROTATION_ANGLE:
            {
                uint16_t current = config_get_rotation_angle();
                if (current == ROTATION_ANGLE_720) config_set_rotation_angle(ROTATION_ANGLE_540);
                else if (current == ROTATION_ANGLE_540) config_set_rotation_angle(ROTATION_ANGLE_360);
                else if (current == ROTATION_ANGLE_360) config_set_rotation_angle(ROTATION_ANGLE_180);
                else if (current == ROTATION_ANGLE_180) config_set_rotation_angle(ROTATION_ANGLE_90);
            }
            break;
        case CONFIG_ITEM_PHOTO_INTERVAL:
            {
                uint8_t current = config_get_photo_interval();
                if (current == PHOTO_INTERVAL_30) config_set_photo_interval(PHOTO_INTERVAL_15);
                else if (current == PHOTO_INTERVAL_15) config_set_photo_interval(PHOTO_INTERVAL_10);
                else if (current == PHOTO_INTERVAL_10) config_set_photo_interval(PHOTO_INTERVAL_5);
            }
            break;
    }
    ui_force_update();
}

/**
 * 获取配置项名称
 */
const char* ui_get_config_item_name(config_item_t item) {
    switch (item) {
        case CONFIG_ITEM_MOTOR_DIRECTION: return "Motor Dir";
        case CONFIG_ITEM_MOTOR_SPEED: return "Motor Speed";
        case CONFIG_ITEM_ROTATION_ANGLE: return "Rotation";
        case CONFIG_ITEM_PHOTO_INTERVAL: return "Photo Int";
        default: return "Unknown";
    }
}

/**
 * 显示配置项值（直接打印，不返回字符串）
 */
void ui_print_config_item_value(config_item_t item) {
    switch (item) {
        case CONFIG_ITEM_MOTOR_DIRECTION:
            display.print(config_get_motor_direction_string());
            break;
        case CONFIG_ITEM_MOTOR_SPEED:
            display.print(config_get_motor_speed());
            display.print(F(" ms"));
            break;
        case CONFIG_ITEM_ROTATION_ANGLE:
            display.print(config_get_rotation_angle_string());
            break;
        case CONFIG_ITEM_PHOTO_INTERVAL:
            display.print(config_get_photo_interval());
            display.print(F(" deg"));
            break;
        default:
            display.print(F("Unknown"));
            break;
    }
}



/**
 * 居中显示文本
 */
void ui_center_text(const char* text, uint8_t y) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

    uint8_t x = (SCREEN_WIDTH - w) / 2;
    display.setCursor(x, y);
    display.print(text);
}
