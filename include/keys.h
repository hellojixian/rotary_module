#ifndef KEYS_H
#define KEYS_H

#include "hal.h"

// 按键函数声明
void keys_init();
void check_keys();
void handle_key_press(int key_num);

// 按键事件处理函数声明
void key0_pressed();
void key1_pressed();
void key2_pressed();
void key3_pressed();

#endif // KEYS_H
