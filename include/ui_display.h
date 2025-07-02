#ifndef UI_DISPLAY_H
#define UI_DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "hal.h"
#include "config.h"
#include "camera.h"
#include "voltage.h"

// 显示区域定义
#define UI_STATUS_BAR_HEIGHT        8
#define UI_SEPARATOR_HEIGHT         1
#define UI_MAIN_CONTENT_HEIGHT      23
#define UI_PROGRESS_BAR_WIDTH       64
#define UI_PROGRESS_BAR_HEIGHT      6

// 配置项索引
typedef enum {
    CONFIG_ITEM_MOTOR_DIRECTION = 0,
    CONFIG_ITEM_MOTOR_SPEED,
    CONFIG_ITEM_ROTATION_ANGLE,
    CONFIG_ITEM_PHOTO_INTERVAL,
    CONFIG_ITEM_COUNT
} config_item_t;

// UI状态
typedef struct {
    bool force_update;              // 强制更新标志
    unsigned long last_update;      // 上次更新时间
    config_item_t current_config_item;  // 当前配置项
    bool in_config_edit;            // 是否在配置编辑模式
} ui_state_t;

// 外部显示对象声明
extern Adafruit_SSD1306 display;

// 函数声明
void ui_init(void);
void ui_update(void);
void ui_force_update(void);

// 状态栏绘制
void ui_draw_status_bar(void);
void ui_draw_camera_status(void);
void ui_draw_battery_status(void);

// 主内容区绘制
void ui_draw_standby(void);
void ui_draw_camera_mode(void);
void ui_draw_scan_mode(void);
void ui_draw_config_menu(void);
void ui_draw_config_edit(void);
void ui_draw_config_menu_fullscreen(void);
void ui_draw_config_edit_fullscreen(void);
void ui_draw_photo_running(uint8_t current_photo, uint8_t total_photos,
                          uint16_t total_angle, uint8_t angle_per_photo);
void ui_draw_scan_running(float turns, unsigned long elapsed_seconds);
void ui_draw_countdown(uint8_t seconds);

// 进度条绘制
void ui_draw_progress_bar(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                         uint8_t current, uint8_t total);

// 配置菜单相关
void ui_config_next_item(void);
void ui_config_prev_item(void);
void ui_config_enter_edit(void);
void ui_config_exit_edit(void);
void ui_config_increase_value(void);
void ui_config_decrease_value(void);
const char* ui_get_config_item_name(config_item_t item);
void ui_print_config_item_value(config_item_t item);

// 辅助函数
void ui_clear_main_area(void);
void ui_draw_separator(void);
void ui_center_text(const char* text, uint8_t y);


#endif // UI_DISPLAY_H
