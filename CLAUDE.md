# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an embedded firmware project for a motorized camera turntable controller built on the LGT8F328P microcontroller (Arduino-compatible). The device controls a 28BYJ-48 stepper motor for rotating objects, integrates with a camera trigger system, and provides a menu-driven UI on an SSD1306 OLED display.

**Target Hardware**: LGT8F328P @ 16MHz with external clock
**Display**: 128x32 OLED (I2C, address 0x3C)
**Motor**: 28BYJ-48 stepper motor with ULN2003 driver
**Camera Interface**: 2.5mm jack with focus/shutter trigger capability

## Build System

This project uses PlatformIO, not Arduino IDE.

### Build Commands
```bash
# Build firmware
pio run

# Upload to board
pio run --target upload

# Clean build artifacts
pio run --target clean

# Open serial monitor (115200 baud)
pio device monitor

# Build and upload in one command
pio run --target upload && pio device monitor
```

### Build Configuration
The `platformio.ini` defines the build environment. Key settings:
- Platform: `lgt8f` (LGT8F328P support)
- Framework: Arduino
- Clock: 16MHz external oscillator
- Libraries: Adafruit GFX and SSD1306 (automatically managed)
- Build flags: `-w` (suppress warnings)

## Architecture

### Hardware Abstraction Layer
All hardware pin definitions are centralized in `include/hal.h`. This includes:
- I2C pins (PC4/SDA, PC5/SCL)
- Key mappings (KEY0-KEY3 mapped to CANCEL/PREV/NEXT/OK)
- Motor control pins (PE0-PE3)
- Camera trigger pins (PD2=detect, PC0=shutter, PC1=focus)
- Timing constants for camera triggers and photo modes

When adding new hardware features, always define pins in `hal.h` first.

### Module Organization

The codebase follows a strict modular architecture with clear separation between hardware drivers, application logic, and UI:

**Core Hardware Modules** (`src/` + `include/`):
- `buzzer`: Tone generation for user feedback
- `keys`: Button debouncing and event detection (rising edge trigger)
- `voltage`: Battery voltage monitoring via ADC with 10K+10K divider
- `camera`: Camera connection detection and trigger control
- `stepper_motor`: 28BYJ-48 motor control with half/full step modes

**Application Logic**:
- `config`: EEPROM-based configuration persistence (motor direction/speed, rotation angle, photo interval)
- `menu_system`: State machine for UI navigation and mode management
- `photo_mode`: Automated panorama photography with configurable intervals
- `scan_mode`: 3D scanning mode (camera-free continuous rotation)

**User Interface**:
- `ui_display`: Display rendering and layout management
- `clock_verify`: CPU clock verification for timing accuracy

### State Machine Architecture

The system uses a hierarchical state machine centered in `menu_system`:

```
MENU_STATE_STANDBY (initial)
├─→ MENU_STATE_CAMERA_MODE (when camera connected)
│   └─→ MENU_STATE_PHOTO_RUNNING (during photo session)
├─→ MENU_STATE_SCAN_MODE (when camera disconnected)
│   └─→ MENU_STATE_SCAN_RUNNING (during scan)
└─→ MENU_STATE_CONFIG
    └─→ MENU_STATE_CONFIG_EDIT (editing parameters)
```

State transitions are triggered by:
1. Camera connection detection (`camera_update_status()`)
2. Key events processed through `menu_handle_key_events()`
3. Mode completion (photo/scan finished)

### Main Loop Architecture

The `loop()` function orchestrates all modules in a non-blocking manner:

```cpp
void loop() {
  keys_update();              // Poll button states
  menu_update();              // Process state machine
  stepper_motor_update();     // Handle motor stepping
  camera_update_status();     // Check camera connection
  camera_update_triggers();   // Non-blocking trigger timing
  photo_mode_update();        // Update photo mode FSM
  scan_mode_update();         // Update scan mode FSM
  update_voltage_reading();   // Periodic voltage check (2s interval)
}
```

All timing is handled via `millis()` - never use blocking delays in module updates.

### Key Event System

Keys use rising edge detection (idle=LOW, pressed=HIGH) with debouncing. The event system provides:
- `KEY_EVENT_PRESS`: Single press
- `KEY_EVENT_LONG_PRESS`: Hold for >1s
- `KEY_EVENT_RELEASE`: Button released

Key handlers are context-dependent - the same physical button performs different actions based on `menu_get_state()`. For example, KEY_OK starts photo mode in CAMERA_MODE but starts scan mode in SCAN_MODE.

### Photo Mode Operation

Photo mode implements a multi-stage state machine for automated panoramic photography:

1. **COUNTDOWN**: 3-second beeping countdown
2. **FOCUS**: Trigger camera focus and wait
3. **PRE_FIRST_SHOT**: Settle time before first photo
4. **FIRST_SHOT**: Capture first image
5. **POST_FIRST_SHOT**: Processing delay
6. **ROTATING**: Move motor by calculated angle
7. **PRE_SHOOTING**: Settle after rotation
8. **SHOOTING**: Capture photo
9. **POST_SHOOTING**: Processing delay
10. **COMPLETE**: Return to menu or loop to ROTATING

Key parameters:
- Rotation angle: 90/180/360/540/720° (configurable)
- Photo interval: 5/10/15/30° (configurable)
- Compensation: +4 steps per stop to account for mechanical settling

### Stepper Motor Control

The 28BYJ-48 uses either half-step (4096 steps/rev, smoother) or full-step (2048 steps/rev, higher torque) sequences. The motor module provides:

- Non-blocking stepping via `stepper_motor_update()`
- Speed control: `motor_speed` parameter (2-30ms delay between steps)
- Direction control: CW/CCW
- Step counting and angle tracking
- Target-based movement: `stepper_motor_rotate_angle()` or `rotate_steps()`

Motor movement is always non-blocking - `stepper_motor_update()` must be called regularly in the main loop.

### Camera Trigger System

Camera detection uses a 3-state model:
1. **Disconnected**: CAMERA_TRIGGER_SENSOR_PIN is HIGH (cable unplugged)
2. **Cable Connected**: Pin is LOW but CAMERA_FOCUS_TRIGGER_PIN is HIGH (no camera body)
3. **Camera Ready**: Both pins LOW (camera body attached)

Triggering is non-blocking:
- Call `camera_trigger_focus()` to initiate
- `camera_update_triggers()` manages timing internally
- Default focus time: 3000ms, shutter time: 3000ms (defined in `hal.h`)

### Configuration System

The `config` module manages persistent settings in EEPROM:
- Magic number validation (0xAB)
- Checksum verification
- Default value restoration on corruption
- Getter/setter functions with validation

Always use `config_set_*()` functions to modify settings, then call `config_save_to_eeprom()` to persist changes.

## Development Guidelines

### Adding New Features

**New Hardware Module**:
1. Define pins in `include/hal.h`
2. Create `include/module_name.h` with public API
3. Create `src/module_name.cpp` with implementation
4. Add `module_name_init()` call in `setup()`
5. Add `module_name_update()` call in `loop()` if periodic updates needed

**New Menu State**:
1. Add enum value to `menu_state_t` in `menu_system.h`
2. Create handler function `menu_handle_new_state()`
3. Add case in `menu_update()` switch statement
4. Define state entry/exit functions if needed

**New Configuration Parameter**:
1. Add field to `system_config_t` in `config.h`
2. Update `EEPROM_CONFIG_SIZE` if struct size changed
3. Add validation functions
4. Add getter/setter functions
5. Update `config_calculate_checksum()` logic
6. Increment `EEPROM_VERSION` to trigger re-initialization

### Testing Without Hardware

The project includes basic tests in `test/` and examples in `examples/camera_test.cpp`. To add tests:

1. Create test file in `test/` directory
2. Use PlatformIO test framework: `pio test`

### Common Pitfalls

- **Clock Timing**: The LGT8F328P requires external clock configuration. If timing is off, check `platformio.ini` clock settings and `clock_verify.cpp` output.
- **I2C Display**: The SSD1306 requires proper I2C initialization. If display doesn't work, verify I2C pins (PC4/PC5) and address (0x3C).
- **Motor Direction**: The 28BYJ-48 has high gear ratio - small step errors accumulate. Use angle compensation (`ANGLE_COMPENSATION_PER_STOP`) for multi-stop movements.
- **Blocking Code**: Never use `delay()` in module update functions - this breaks the non-blocking architecture. Use `millis()` timers instead.
- **EEPROM Wear**: EEPROM has limited write cycles (~100k). Only save configuration on user request, not on every change.

### Debugging

Serial output is available at 115200 baud:
```cpp
Serial.println("Debug message");
```

The `clock_verify()` function in `setup()` prints CPU clock diagnostics to help identify timing issues.

## Code Style

- Pin definitions: UPPERCASE with descriptive names
- Functions: snake_case with module prefix (e.g., `stepper_motor_init()`)
- Types: snake_case with `_t` suffix (e.g., `menu_state_t`)
- Enums: UPPERCASE values (e.g., `MENU_STATE_STANDBY`)
- Global variables: `g_` prefix (e.g., `g_config`)
- Constants: UPPERCASE with `#define`

## Important Hardware Constraints

- **Voltage Range**: 0-10V via divider on PC2 ADC
- **Motor Current**: ULN2003 can handle 500mA per channel, but avoid prolonged activation to prevent overheating
- **Camera Triggers**: Open-drain outputs (INPUT_PULLUP when idle, OUTPUT_LOW when triggering)
- **Display Update Rate**: Limit OLED updates to prevent I2C bus congestion
- **Memory**: LGT8F328P has 32KB flash, 2KB RAM - avoid large buffers or strings
