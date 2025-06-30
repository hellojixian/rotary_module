# 项目模块化结构说明

## 项目概述
本项目已经完全模块化，将原来的单一main.cpp文件拆分成多个功能模块，提高了代码的可维护性和可扩展性。

## 文件结构

### 头文件 (include/)
- `hal.h` - 硬件抽象层定义，包含引脚定义和常量
- `buzzer.h` - 蜂鸣器模块头文件
- `keys.h` - 按键模块头文件
- `voltage.h` - 电压监测模块头文件
- `camera.h` - 相机状态检测模块头文件

### 源文件 (src/)
- `main.cpp` - 主程序文件，负责系统初始化和主循环
- `buzzer.cpp` - 蜂鸣器功能实现
- `keys.cpp` - 按键检测和处理逻辑
- `voltage.cpp` - 电压监测功能实现
- `camera.cpp` - 相机状态检测功能实现
- `key0_event.cpp` - 按键0的事件处理
- `key1_event.cpp` - 按键1的事件处理
- `key2_event.cpp` - 按键2的事件处理（相机对焦）
- `key3_event.cpp` - 按键3的事件处理（相机快门）

## 模块功能说明

### 1. 硬件抽象层 (hal.h)
- 定义所有硬件引脚
- 定义系统常量
- 定义音符频率常量

### 2. 蜂鸣器模块 (buzzer.h/cpp)
**功能：**
- `buzzer_init()` - 蜂鸣器初始化
- `buzzer_tone(frequency, duration)` - 播放指定频率和时长的音调
- `play_startup_melody()` - 播放启动旋律

### 3. 按键模块 (keys.h/cpp)
**功能：**
- `keys_init()` - 按键初始化（寄存器配置）
- `check_keys()` - 检查按键状态（上升沿检测）
- `handle_key_press(key_num)` - 处理按键按下事件

**按键配置：**
- 默认低电平，按下时高电平
- 使用寄存器直接读取（PIND, PINE）
- 上升沿触发检测

### 4. 电压监测模块 (voltage.h/cpp)
**功能：**
- `voltage_sensor_init()` - 电压传感器初始化
- `read_battery_voltage()` - 读取电池电压
- `display_battery_voltage()` - 在屏幕右上角显示电压
- `update_voltage_display()` - 每2秒更新电压显示

**硬件配置：**
- 使用PC2引脚（ADC）
- 10K+10K分压器
- 测量范围：0-10V

### 5. 相机状态检测模块 (camera.h/cpp)
**功能：**
- `camera_init()` - 相机模块初始化
- `camera_update_status()` - 更新相机连接状态
- `camera_get_status()` - 获取当前相机状态
- `camera_display_status()` - 在屏幕上显示相机状态
- `camera_trigger_focus()` - 触发相机对焦
- `camera_trigger_shutter()` - 触发相机快门

**状态检测：**
- 完全未连接：连接线未插入
- 连接线已链接：连接线插入但未检测到相机
- 已经正常链接：连接线插入且检测到相机

**硬件配置：**
- CAMERA_TRIGGER_SENSOR_PIN (PD2)：检测连接线插入（INPUT_PULLUP）
- CAMERA_FOCUS_TRIGGER_PIN (PC1)：检测相机/触发对焦（INPUT_PULLUP/OUTPUT_LOW）
- CAMERA_SHUTTER_TRIGGER_PIN (PC0)：触发快门（INPUT_PULLUP/OUTPUT_LOW）

### 6. 按键事件处理模块
每个按键都有独立的事件处理文件：

**key0_event.cpp:**
- `key0_pressed()` - 按键0按下时的处理逻辑
- 功能：步进电机低速转动控制
- 播放1800Hz提示音

**key1_event.cpp:**
- `key1_pressed()` - 按键1按下时的处理逻辑
- 功能：步进电机高速转动控制
- 播放1900Hz提示音

**key2_event.cpp:**
- `key2_pressed()` - 按键2按下时的处理逻辑
- 功能：相机对焦控制
- 播放2000Hz提示音

**key3_event.cpp:**
- `key3_pressed()` - 按键3按下时的处理逻辑
- 功能：相机对焦控制
- 播放2100Hz提示音

## 主程序流程 (main.cpp)

### setup()函数：
1. 初始化串口通信
2. 初始化I2C总线
3. 初始化OLED显示屏
4. 初始化各个功能模块（蜂鸣器、按键、电压、步进电机、相机）
5. 播放启动旋律
6. 读取初始电池电压

### loop()函数：
1. 检查按键状态
2. 更新步进电机状态
3. 更新相机状态
4. 更新电压显示
5. 短暂延时

## 模块间依赖关系

```
main.cpp
├── hal.h (硬件定义)
├── buzzer.h/cpp (蜂鸣器功能)
├── keys.h/cpp (按键检测)
│   ├── key0_event.cpp (步进电机控制)
│   ├── key1_event.cpp (步进电机控制)
│   ├── key2_event.cpp (相机对焦)
│   └── key3_event.cpp (相机对焦)
├── voltage.h/cpp (电压监测)
├── stepper_motor.h/cpp (步进电机控制)
└── camera.h/cpp (相机状态检测)
```

## 扩展说明

### 添加新按键功能：
1. 在对应的`keyX_event.cpp`文件中修改`keyX_pressed()`函数
2. 可以调用其他模块的功能（如蜂鸣器、显示等）

### 添加新模块：
1. 创建对应的`.h`和`.cpp`文件
2. 在`main.cpp`中包含头文件
3. 在`setup()`中调用初始化函数
4. 在`loop()`中调用更新函数（如需要）

### 修改硬件配置：
1. 在`hal.h`中修改引脚定义
2. 相关模块会自动使用新的定义

## 编译说明
确保PlatformIO项目配置正确，所有源文件都会被自动编译和链接。

## 优势
1. **模块化设计** - 每个功能独立，便于维护
2. **可扩展性** - 容易添加新功能
3. **代码复用** - 模块可以在其他项目中复用
4. **调试方便** - 问题定位更精确
5. **团队协作** - 不同人员可以负责不同模块
