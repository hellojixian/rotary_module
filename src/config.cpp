#include "config.h"

// 全局配置变量
system_config_t g_config;

/**
 * 初始化配置系统
 */
void config_init(void) {
    // 尝试从EEPROM加载配置
    config_load_from_eeprom();

    // 验证配置
    if (!config_validate()) {
        config_reset_to_default();
        config_save_to_eeprom();
    }
}

/**
 * 从EEPROM加载配置
 */
void config_load_from_eeprom(void) {
    EEPROM.get(EEPROM_CONFIG_START_ADDR, g_config);
}

/**
 * 保存配置到EEPROM
 */
void config_save_to_eeprom(void) {
    // 计算校验和
    g_config.checksum = config_calculate_checksum(&g_config);

    // 保存到EEPROM
    EEPROM.put(EEPROM_CONFIG_START_ADDR, g_config);

    Serial.println(F("Configuration saved to EEPROM"));
}

/**
 * 重置为默认配置
 */
void config_reset_to_default(void) {
    g_config.magic = EEPROM_MAGIC_NUMBER;
    g_config.version = EEPROM_VERSION;
    g_config.motor_direction = MOTOR_DIRECTION_CW;
    g_config.motor_speed = MOTOR_SPEED_DEFAULT;
    g_config.rotation_angle = ROTATION_ANGLE_DEFAULT;
    g_config.photo_interval = PHOTO_INTERVAL_DEFAULT;
    g_config.checksum = 0; // 将在保存时计算

    Serial.println(F("Configuration reset to defaults"));
}

/**
 * 验证配置有效性
 */
bool config_validate(void) {
    // 检查魔数和版本
    if (g_config.magic != EEPROM_MAGIC_NUMBER || g_config.version != EEPROM_VERSION) {
        return false;
    }

    // 检查校验和
    uint8_t calculated_checksum = config_calculate_checksum(&g_config);
    if (g_config.checksum != calculated_checksum) {
        return false;
    }

    // 检查参数范围
    if (!config_is_valid_motor_direction(g_config.motor_direction) ||
        !config_is_valid_motor_speed(g_config.motor_speed) ||
        !config_is_valid_rotation_angle(g_config.rotation_angle) ||
        !config_is_valid_photo_interval(g_config.photo_interval)) {
        return false;
    }

    return true;
}

/**
 * 计算配置校验和
 */
uint8_t config_calculate_checksum(const system_config_t* config) {
    uint8_t checksum = 0;
    const uint8_t* data = (const uint8_t*)config;

    // 计算除校验和字段外的所有字节
    for (int i = 0; i < sizeof(system_config_t) - 1; i++) {
        checksum ^= data[i];
    }

    return checksum;
}

/**
 * 获取配置指针
 */
system_config_t* config_get(void) {
    return &g_config;
}

/**
 * 获取电机方向
 */
uint8_t config_get_motor_direction(void) {
    return g_config.motor_direction;
}

/**
 * 获取电机速度
 */
uint8_t config_get_motor_speed(void) {
    return g_config.motor_speed;
}

/**
 * 获取旋转角度
 */
uint16_t config_get_rotation_angle(void) {
    return g_config.rotation_angle;
}

/**
 * 获取拍照间隔
 */
uint8_t config_get_photo_interval(void) {
    return g_config.photo_interval;
}

/**
 * 设置电机方向
 */
void config_set_motor_direction(uint8_t direction) {
    if (config_is_valid_motor_direction(direction)) {
        g_config.motor_direction = direction;
    }
}

/**
 * 设置电机速度
 */
void config_set_motor_speed(uint8_t speed) {
    if (config_is_valid_motor_speed(speed)) {
        g_config.motor_speed = speed;
    }
}

/**
 * 设置旋转角度
 */
void config_set_rotation_angle(uint16_t angle) {
    if (config_is_valid_rotation_angle(angle)) {
        g_config.rotation_angle = angle;
    }
}

/**
 * 设置拍照间隔
 */
void config_set_photo_interval(uint8_t interval) {
    if (config_is_valid_photo_interval(interval)) {
        g_config.photo_interval = interval;
    }
}

/**
 * 验证电机方向
 */
bool config_is_valid_motor_direction(uint8_t direction) {
    return (direction == MOTOR_DIRECTION_CW || direction == MOTOR_DIRECTION_CCW);
}

/**
 * 验证电机速度
 */
bool config_is_valid_motor_speed(uint8_t speed) {
    // 检查是否为预设的有效值
    const uint8_t valid_speeds[] = {2, 4, 6, 8, 10, 15, 30, 60, 100};
    const uint8_t valid_count = sizeof(valid_speeds) / sizeof(valid_speeds[0]);

    for (uint8_t i = 0; i < valid_count; i++) {
        if (speed == valid_speeds[i]) {
            return true;
        }
    }
    return false;
}

/**
 * 验证旋转角度
 */
bool config_is_valid_rotation_angle(uint16_t angle) {
    return (angle == ROTATION_ANGLE_90 || angle == ROTATION_ANGLE_180 ||
            angle == ROTATION_ANGLE_360 || angle == ROTATION_ANGLE_540 ||
            angle == ROTATION_ANGLE_720);
}

/**
 * 验证拍照间隔
 */
bool config_is_valid_photo_interval(uint8_t interval) {
    return (interval == PHOTO_INTERVAL_5 || interval == PHOTO_INTERVAL_10 ||
            interval == PHOTO_INTERVAL_15 || interval == PHOTO_INTERVAL_30);
}

/**
 * 获取电机方向字符串
 */
const char* config_get_motor_direction_string(void) {
    return (g_config.motor_direction == MOTOR_DIRECTION_CW) ? "CW" : "CCW";
}

/**
 * 获取旋转角度字符串
 */
const char* config_get_rotation_angle_string(void) {
    switch (g_config.rotation_angle) {
        case ROTATION_ANGLE_90:  return "90 deg";
        case ROTATION_ANGLE_180: return "180 deg";
        case ROTATION_ANGLE_360: return "360 deg";
        case ROTATION_ANGLE_540: return "540 deg";
        case ROTATION_ANGLE_720: return "720 deg";
        default: return "Unknown";
    }
}
