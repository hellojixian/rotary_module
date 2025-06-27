# LGT8FX8P 16MHz外部时钟配置指南

## 概述

本文档说明如何在LGT8FX8P项目中启用16MHz外部时钟，以及如何使用独立的时钟验证模块。

## 文件结构

### 新增文件

1. **include/clock_verify.h** - 时钟验证模块头文件
2. **src/clock_verify.cpp** - 时钟验证模块实现文件
3. **docs/clock_configuration_guide.md** - 本配置指南

### 修改文件

1. **platformio.ini** - 添加了16MHz外部时钟配置
2. **src/main.cpp** - 移除了内联的verify_clock函数，改为调用独立模块

## 配置详情

### PlatformIO配置 (platformio.ini)

```ini
; 时钟配置
board_build.f_cpu = 16000000L  ; 设置CPU频率为16MHz
board_build.f_osc = 16000000L  ; 设置振荡器频率为16MHz
board_build.clock_source = 2   ; 外部时钟源
build_flags =
    -DF_CPU=16000000L          ; 编译时定义CPU频率
    -DCLOCK_SOURCE=2           ; 编译时定义时钟源类型
    -w                         ; 禁用警告
```

### 时钟源定义

- `CLOCK_SOURCE=1` : 内部时钟
- `CLOCK_SOURCE=2` : 外部时钟

## 时钟验证模块

### 功能特性

1. **verify_clock()** - 主验证函数，打印完整的时钟信息
2. **print_clock_info()** - 打印CPU频率配置信息
3. **get_current_cpu_frequency()** - 获取当前CPU频率
4. **get_clock_source_name()** - 获取时钟源名称

### 使用方法

在main.cpp中调用：

```cpp
#include "clock_verify.h"

void setup() {
    Serial.begin(115200);
    
    // 验证时钟配置
    verify_clock();
    
    // 其他初始化代码...
}
```

### 输出示例

```
=== Clock Verification ===
Configured F_CPU: 16MHz
Current CPU Frequency: 16 MHz
Clock Source: External Clock
=========================
```

## 硬件要求

1. **16MHz晶振** - 连接到LGT8FX8P的XTAL1和XTAL2引脚
2. **负载电容** - 通常使用22pF电容连接到地
3. **PCB布线** - 晶振应尽可能靠近芯片，走线要短

## 编译验证

项目已成功编译，内存使用情况：
- RAM: 53.6% (1098/2048 bytes)
- Flash: 62.8% (18660/29696 bytes)

## 故障排除

### 常见问题

1. **时钟不稳定**
   - 检查晶振连接
   - 确认负载电容值正确
   - 检查PCB布线质量

2. **编译错误**
   - 确保platformio.ini配置正确
   - 检查头文件包含路径

3. **运行时问题**
   - 使用串口监视器查看时钟验证输出
   - 确认F_CPU和CLOCK_SOURCE定义正确

### 调试命令

```bash
# 编译项目
pio run

# 上传并监视串口输出
pio run --target upload --target monitor
```

## 注意事项

1. 外部时钟配置需要硬件支持，确保电路设计正确
2. 时钟切换可能影响现有的定时器和延时函数
3. 建议在更改时钟配置后重新校准所有时序相关的功能
4. 某些LGT8FX8P变种可能需要不同的配置参数

## 版本历史

- v1.0 - 初始版本，实现基本的16MHz外部时钟配置和验证功能
