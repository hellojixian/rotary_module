#ifndef VOLTAGE_H
#define VOLTAGE_H

#include "hal.h"

// 电压监测函数声明
void voltage_sensor_init();
float read_battery_voltage();
void display_battery_voltage();
void update_voltage_reading();

// 电压监测相关变量声明
extern unsigned long last_voltage_check;
extern float battery_voltage;

#endif // VOLTAGE_H
