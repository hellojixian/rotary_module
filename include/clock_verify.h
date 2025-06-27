#ifndef CLOCK_VERIFY_H
#define CLOCK_VERIFY_H

#include <Arduino.h>

// 时钟验证相关函数声明
void verify_clock(void);
void print_clock_info(void);
uint32_t get_current_cpu_frequency(void);
const char* get_clock_source_name(void);

// 时钟源定义
#define CLOCK_SOURCE_INTERNAL 1
#define CLOCK_SOURCE_EXTERNAL 2

#endif // CLOCK_VERIFY_H
