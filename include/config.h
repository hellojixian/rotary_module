#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <EEPROM.h>

// EEPROM存储配置
#define EEPROM_CONFIG_START_ADDR    0
#define EEPROM_MAGIC_NUMBER         0xAB
#define EEPROM_VERSION              1
#define EEPROM_CONFIG_SIZE          8

// 配置参数范围定义
#define MOTOR_DIRECTION_CW          0
#define MOTOR_DIRECTION_CCW         1

#define MOTOR_SPEED_MIN             2
#define MOTOR_SPEED_MAX             8
#define MOTOR_SPEED_DEFAULT         4

#define ROTATION_ANGLE_90           90
#define ROTATION_ANGLE_180          180
#define ROTATION_ANGLE_360          360
#define ROTATION_ANGLE_540          540
#define ROTATION_ANGLE_720          720
#define ROTATION_ANGLE_DEFAULT      ROTATION_ANGLE_360

#define PHOTO_INTERVAL_5            5
#define PHOTO_INTERVAL_10           10
#define PHOTO_INTERVAL_15           15
#define PHOTO_INTERVAL_30           30
#define PHOTO_INTERVAL_DEFAULT      PHOTO_INTERVAL_15

// 配置结构体
typedef struct {
    uint8_t magic;              // 魔数，用于验证配置有效性
    uint8_t version;            // 版本号
    uint8_t motor_direction;    // 电机方向：0=顺时针，1=逆时针
    uint8_t motor_speed;        // 电机速度：2-8ms
    uint16_t rotation_angle;    // 旋转角度：90/180/360/540/720度
    uint8_t photo_interval;     // 拍照间隔：5/10/15/30度
    uint8_t checksum;           // 校验和
} system_config_t;

// 全局配置变量
extern system_config_t g_config;

// 函数声明
void config_init(void);
void config_load_from_eeprom(void);
void config_save_to_eeprom(void);
void config_reset_to_default(void);
bool config_validate(void);
uint8_t config_calculate_checksum(const system_config_t* config);

// 配置获取函数
system_config_t* config_get(void);
uint8_t config_get_motor_direction(void);
uint8_t config_get_motor_speed(void);
uint16_t config_get_rotation_angle(void);
uint8_t config_get_photo_interval(void);

// 配置设置函数
void config_set_motor_direction(uint8_t direction);
void config_set_motor_speed(uint8_t speed);
void config_set_rotation_angle(uint16_t angle);
void config_set_photo_interval(uint8_t interval);

// 配置验证函数
bool config_is_valid_motor_direction(uint8_t direction);
bool config_is_valid_motor_speed(uint8_t speed);
bool config_is_valid_rotation_angle(uint16_t angle);
bool config_is_valid_photo_interval(uint8_t interval);

// 配置字符串转换函数（用于显示）
const char* config_get_motor_direction_string(void);
const char* config_get_rotation_angle_string(void);

#endif // CONFIG_H
