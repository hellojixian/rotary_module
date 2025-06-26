# 28BYJ-48 步进电机驱动使用指南

## 概述

本驱动程序为28BYJ-48步进电机提供完整的控制功能，通过ULN2003APG控制器连接到微控制器。支持速度控制、方向控制和精确的角度控制。

## 硬件连接

### 引脚连接
```
微控制器    ULN2003APG    28BYJ-48
PE0    ->   INT1      ->   线圈1
PE1    ->   INT2      ->   线圈2  
PE2    ->   INT3      ->   线圈3
PE3    ->   INT4      ->   线圈4
```

### 电源连接
- ULN2003APG VCC: 5V
- ULN2003APG GND: GND
- 28BYJ-48 红线: 5V
- 28BYJ-48 其他线: 连接到ULN2003APG输出

## 软件使用

### 1. 包含头文件
```cpp
#include "stepper_motor.h"
```

### 2. 初始化
```cpp
void setup() {
    stepper_motor_init();
}
```

### 3. 主循环更新
```cpp
void loop() {
    stepper_motor_update();  // 必须在主循环中调用
    // 其他代码...
}
```

## API 参考

### 基本控制函数

#### `void stepper_motor_init()`
初始化步进电机，设置引脚为输出模式并初始化电机状态。

#### `void stepper_motor_set_speed(motor_speed_t speed)`
设置电机转动速度。
- `SPEED_LOW`: 低速 (3ms延时)
- `SPEED_HIGH`: 高速 (1ms延时)

#### `void stepper_motor_set_direction(motor_direction_t direction)`
设置电机转动方向。
- `CLOCKWISE`: 顺时针
- `COUNTER_CLOCKWISE`: 逆时针

#### `void stepper_motor_start()`
启动电机连续转动。

#### `void stepper_motor_stop()`
停止电机转动。

#### `bool stepper_motor_is_running()`
检查电机是否正在运行。

### 精确控制函数

#### `void stepper_motor_rotate_angle(float angle)`
按指定角度旋转电机。
- `angle`: 角度值（度），正数为顺时针，负数为逆时针
- 示例: `stepper_motor_rotate_angle(90.0)` // 顺时针转90度

#### `void stepper_motor_rotate_steps(int steps)`
按指定步数旋转电机。
- `steps`: 步数，必须为正数
- 示例: `stepper_motor_rotate_steps(512)` // 转512步（约90度）

#### `void stepper_motor_update()`
更新电机状态，必须在主循环中定期调用。

## 使用示例

### 示例1: 按键控制
```cpp
#include "stepper_motor.h"
#include "hal.h"

void setup() {
    stepper_motor_init();
    pinMode(KEY0_PIN, INPUT_PULLUP);
    pinMode(KEY1_PIN, INPUT_PULLUP);
}

void loop() {
    // Key0: 低速转动
    if (digitalRead(KEY0_PIN) == LOW) {
        if (!stepper_motor_is_running()) {
            stepper_motor_set_speed(SPEED_LOW);
            stepper_motor_start();
        }
    }
    
    // Key1: 高速转动
    if (digitalRead(KEY1_PIN) == LOW) {
        if (!stepper_motor_is_running()) {
            stepper_motor_set_speed(SPEED_HIGH);
            stepper_motor_start();
        }
    }
    
    stepper_motor_update();
    delay(10);
}
```

### 示例2: 精确角度控制
```cpp
void demo_angle_control() {
    // 顺时针转90度
    stepper_motor_rotate_angle(90);
    
    // 等待转动完成
    while (stepper_motor_is_running()) {
        stepper_motor_update();
        delay(10);
    }
    
    delay(1000);  // 暂停1秒
    
    // 逆时针转180度
    stepper_motor_rotate_angle(-180);
    
    // 等待转动完成
    while (stepper_motor_is_running()) {
        stepper_motor_update();
        delay(10);
    }
}
```

### 示例3: 连续转动控制
```cpp
void demo_continuous_rotation() {
    // 设置高速顺时针连续转动
    stepper_motor_set_speed(SPEED_HIGH);
    stepper_motor_set_direction(CLOCKWISE);
    stepper_motor_start();
    
    // 转动5秒
    unsigned long start_time = millis();
    while (millis() - start_time < 5000) {
        stepper_motor_update();
        delay(10);
    }
    
    // 停止转动
    stepper_motor_stop();
}
```

## 技术参数

### 28BYJ-48 电机参数
- 步进角度: 5.625°/64 = 0.087890625° (半步模式)
- 减速比: 1:64
- 每转步数: 2048步 (半步模式)
- 工作电压: 5V DC
- 工作电流: ≤ 20mA

### 驱动参数
- 步进模式: 8步半步模式
- 低速延时: 3000μs (约333步/秒)
- 高速延时: 1000μs (约1000步/秒)
- 控制精度: 0.176°/步

## 注意事项

1. **电源要求**: 确保5V电源能提供足够电流（建议≥500mA）
2. **散热**: 长时间运行时注意ULN2003APG的散热
3. **机械负载**: 避免超过电机的额定扭矩
4. **更新频率**: 必须在主循环中定期调用`stepper_motor_update()`
5. **引脚冲突**: 确保PE0-PE3引脚没有被其他功能占用

## 故障排除

### 电机不转动
1. 检查电源连接
2. 检查引脚连接
3. 确认`stepper_motor_update()`被调用
4. 检查电机是否卡死

### 转动不平稳
1. 检查电源电压是否稳定
2. 降低转动速度
3. 检查机械连接是否松动

### 转动方向错误
1. 检查线序连接
2. 使用`stepper_motor_set_direction()`设置正确方向

## 扩展功能

可以根据需要扩展以下功能：
- 加速/减速控制
- 微步控制
- 位置反馈
- 多电机同步控制
- PWM调速

## 版本历史

- v1.0: 基础功能实现
  - 速度控制
  - 方向控制  
  - 角度控制
  - 按键控制接口
