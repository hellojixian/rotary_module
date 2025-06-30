#include <Arduino.h>
#include "keys.h"

// 按键状态数组
static key_state_t key_states[KEY_NUM_COUNT];

/**
 * 按键初始化函数
 */
void keys_init(void) {
    // 设置 PE4 和 PE5 为输入（0）不开启上拉（0）
    DDRE &= ~((1 << KEY2_PIN) | (1 << KEY3_PIN));     // 输入模式
    // PORTE &= ~((1 << KEY2_PIN) | (1 << KEY3_PIN));    // 不开启上拉电阻

    // 设置 PD3 和 PD4 为输入（0）不开启上拉（0）
    DDRD &= ~((1 << KEY0_PIN) | (1 << KEY1_PIN));     // 输入模式
    // PORTD &= ~((1 << KEY0_PIN) | (1 << KEY1_PIN));    // 不开启上拉电阻

    // 初始化按键状态
    for (int i = 0; i < KEY_NUM_COUNT; i++) {
        key_states[i].current_state = false;
        key_states[i].last_state = false;
        key_states[i].press_start_time = 0;
        key_states[i].long_press_triggered = false;
    }


}

/**
 * 更新按键状态（需要在主循环中调用）
 */
void keys_update(void) {
    unsigned long current_time = millis();

    // 更新所有按键状态
    for (int i = 0; i < KEY_NUM_COUNT; i++) {
        bool hardware_state = keys_read_hardware_state((key_num_t)i);
        keys_update_key_state((key_num_t)i, hardware_state);
    }
}

/**
 * 获取按键事件
 */
key_event_t keys_get_event(key_num_t key) {
    if (key >= KEY_NUM_COUNT) return KEY_EVENT_NONE;

    key_state_t* state = &key_states[key];
    unsigned long current_time = millis();

    // 检查短按事件（上升沿触发）
    if (state->last_state == false && state->current_state == true) {
        // 按键刚按下，记录时间
        state->press_start_time = current_time;
        state->long_press_triggered = false;
        return KEY_EVENT_NONE; // 等待确定是短按还是长按
    }

    // 检查长按事件
    if (state->current_state == true && !state->long_press_triggered) {
        if (current_time - state->press_start_time >= LONG_PRESS_TIME_MS) {
            state->long_press_triggered = true;
            return KEY_EVENT_LONG_PRESS;
        }
    }

    // 检查短按事件（下降沿触发，且未触发长按）
    if (state->last_state == true && state->current_state == false) {
        if (!state->long_press_triggered) {
            return KEY_EVENT_SHORT_PRESS;
        }
    }

    return KEY_EVENT_NONE;
}

/**
 * 检查按键是否被按下
 */
bool keys_is_pressed(key_num_t key) {
    if (key >= KEY_NUM_COUNT) return false;
    return key_states[key].current_state;
}

/**
 * 更新单个按键状态
 */
void keys_update_key_state(key_num_t key, bool current_state) {
    if (key >= KEY_NUM_COUNT) return;

    key_state_t* state = &key_states[key];
    state->last_state = state->current_state;
    state->current_state = current_state;
}

/**
 * 读取硬件按键状态
 */
bool keys_read_hardware_state(key_num_t key) {
    switch (key) {
        case KEY_NUM_CANCEL:  // KEY0
            return (PIND & (1 << KEY0_PIN)) != 0;
        case KEY_NUM_PREV:    // KEY1
            return (PIND & (1 << KEY1_PIN)) != 0;
        case KEY_NUM_NEXT:    // KEY2
            return (PINE & (1 << KEY2_PIN)) != 0;
        case KEY_NUM_OK:      // KEY3
            return (PINE & (1 << KEY3_PIN)) != 0;
        default:
            return false;
    }
}
