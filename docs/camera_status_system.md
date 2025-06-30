# 相机状态检测系统

## 概述

本文档描述了新增的相机状态检测系统，该系统能够检测相机连接线的插入状态以及相机的连接状态，并提供三种不同的状态指示。

## 系统状态

系统支持以下三种相机连接状态：

### 1. 完全未连接 (CAMERA_DISCONNECTED)
- **描述**: 相机连接线未插入
- **检测条件**: `CAMERA_TRIGGER_SENSOR_PIN` (PD2) 保持高电平（上拉状态）
- **显示**: 屏幕显示 "CAM: OFF"

### 2. 连接线已链接但未检测到相机 (CAMERA_CABLE_CONNECTED)
- **描述**: 连接线已插入，但未检测到相机
- **检测条件**: 
  - `CAMERA_TRIGGER_SENSOR_PIN` (PD2) 变为低电平（连接线插入）
  - `CAMERA_FOCUS_TRIGGER_PIN` (PC1) 保持低电平（未检测到相机）
- **显示**: 屏幕显示 "CAM: CABLE"

### 3. 已经正常链接到相机 (CAMERA_FULLY_CONNECTED)
- **描述**: 连接线已插入且检测到相机
- **检测条件**: 
  - `CAMERA_TRIGGER_SENSOR_PIN` (PD2) 为低电平（连接线插入）
  - `CAMERA_FOCUS_TRIGGER_PIN` (PC1) 变为高电平（检测到相机）
- **显示**: 屏幕显示 "CAM: READY"

## 硬件配置

### GPIO引脚配置

#### CAMERA_TRIGGER_SENSOR_PIN (PD2)
- **功能**: 检测连接线是否插入
- **配置**: INPUT_PULLUP（输入模式，开启上拉电阻）
- **逻辑**: 
  - 高电平 = 连接线未插入
  - 低电平 = 连接线已插入

#### CAMERA_FOCUS_TRIGGER_PIN (PC1)
- **功能**: 检测相机连接状态 / 触发相机对焦
- **配置**:
  - 默认: INPUT_PULLUP（输入模式，开启上拉电阻，高电平）
  - 触发时: OUTPUT LOW（输出低电平）
- **逻辑**:
  - 低电平 = 未检测到相机（仅连接线插入）
  - 高电平 = 检测到相机（相机连接后提供高电平信号）

#### CAMERA_SHUTTER_TRIGGER_PIN (PC0)
- **功能**: 触发相机快门
- **配置**:
  - 默认: INPUT_PULLUP（输入模式，开启上拉电阻，高电平）
  - 触发时: OUTPUT LOW（输出低电平）

## 文件结构

### 新增文件
```
include/camera.h          # 相机模块头文件
src/camera.cpp           # 相机模块实现文件
docs/camera_status_system.md  # 本文档
```

### 修改文件
```
src/main.cpp             # 添加相机模块初始化和更新
src/voltage.cpp          # 添加相机状态显示
src/key2_event.cpp       # 添加相机对焦控制
src/key3_event.cpp       # 添加相机快门控制
```

## API接口

### 初始化和状态更新
```cpp
void camera_init(void);                    // 初始化相机模块
void camera_update_status(void);           // 更新相机状态（需在主循环中调用）
```

### 状态查询
```cpp
camera_status_t camera_get_status(void);   // 获取当前相机状态
const char* camera_get_status_string(void); // 获取状态字符串
```

### 显示功能
```cpp
void camera_display_status(void);          // 在屏幕上显示相机状态
```

### 相机控制
```cpp
void camera_trigger_focus(void);           // 触发相机对焦
void camera_trigger_shutter(void);         // 触发相机快门
void camera_release_triggers(void);        // 释放所有触发信号
```

## 按键功能

### 按键2 (KEY2) - 相机对焦
- **功能**: 触发相机对焦
- **条件**: 仅在相机完全连接状态下有效
- **操作**:
  1. 设置 `CAMERA_FOCUS_TRIGGER_PIN` 为输出低电平
  2. 保持100ms
  3. 恢复为输入上拉模式

### 按键3 (KEY3) - 相机对焦
- **功能**: 触发相机对焦
- **条件**: 仅在相机完全连接状态下有效
- **操作**:
  1. 设置 `CAMERA_FOCUS_TRIGGER_PIN` 为输出低电平
  2. 保持100ms
  3. 恢复为输入上拉模式

## 状态检测逻辑

### 连接线检测
```cpp
bool camera_check_cable_connection(void) {
    bool current_state = (PIND & (1 << CAMERA_TRIGGER_SENSOR_PIN)) != 0;
    
    // 检测从高电平到低电平的变化（连接线插入）
    if (trigger_sensor_last_state && !current_state) {
        cable_connected = true;
    }
    // 检测从低电平到高电平的变化（连接线拔出）
    else if (!trigger_sensor_last_state && current_state) {
        cable_connected = false;
        camera_detected = false;  // 连接线拔出时重置相机检测
    }
    
    return cable_connected;
}
```

### 相机检测
```cpp
bool camera_check_camera_detection(void) {
    if (!cable_connected) return false;  // 连接线未插入时不检测相机

    bool current_state = (PINC & (1 << CAMERA_FOCUS_TRIGGER_PIN)) != 0;

    // 直接根据当前状态判断相机是否连接
    // 高电平 = 相机连接，低电平 = 相机未连接
    if (current_state && !camera_detected) {
        camera_detected = true;
    }
    else if (!current_state && camera_detected) {
        camera_detected = false;
    }

    return camera_detected;
}
```

## 集成说明

### 主程序集成
1. 在 `setup()` 中调用 `camera_init()`
2. 在 `loop()` 中调用 `camera_update_status()`
3. 在显示更新中调用 `camera_display_status()`

### 状态更新频率
- 相机状态检测: 每100ms检查一次
- 显示更新: 跟随电压显示更新（每2秒）

## 调试信息

系统会通过串口输出以下调试信息：
- `"Camera cable connected"` - 连接线插入
- `"Camera cable disconnected"` - 连接线拔出
- `"Camera detected"` - 检测到相机
- `"Camera connection lost"` - 相机连接丢失
- `"Camera status changed to: [状态]"` - 状态变化
- `"Camera focus triggered"` - 对焦触发
- `"Camera shutter triggered"` - 快门触发
- `"Camera triggers released"` - 触发信号释放

## 注意事项

1. **GPIO控制**: 所有GPIO操作都通过直接寄存器操作实现，确保精确控制
2. **状态同步**: 相机检测只在连接线插入后进行，避免误检测
3. **触发时序**: 快门触发前会先进行对焦，确保拍照质量
4. **错误处理**: 在相机未准备好时尝试触发会输出错误信息
5. **资源管理**: 触发后会自动释放GPIO资源，恢复为输入模式
