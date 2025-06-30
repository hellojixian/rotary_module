#ifndef KEYS_H
#define KEYS_H

#include "hal.h"

// 长按检测时间（毫秒）
#define LONG_PRESS_TIME_MS      1000

// 按键状态结构体
typedef struct {
    bool current_state;
    bool last_state;
    unsigned long press_start_time;
    bool long_press_triggered;
} key_state_t;

// 按键事件类型
typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_SHORT_PRESS,
    KEY_EVENT_LONG_PRESS
} key_event_t;

// 按键编号
typedef enum {
    KEY_NUM_CANCEL = 0,     // KEY0 - CANCEL
    KEY_NUM_PREV = 1,       // KEY1 - PREV
    KEY_NUM_NEXT = 2,       // KEY2 - NEXT
    KEY_NUM_OK = 3,         // KEY3 - OK
    KEY_NUM_COUNT
} key_num_t;

// 函数声明
void keys_init(void);
void keys_update(void);
key_event_t keys_get_event(key_num_t key);
bool keys_is_pressed(key_num_t key);

// 内部函数
void keys_update_key_state(key_num_t key, bool current_state);
bool keys_read_hardware_state(key_num_t key);

#endif // KEYS_H
