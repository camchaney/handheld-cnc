# Handheld CNC Firmware - AI Agent Guide

## Project Overview

**Compass** is a DIY handheld CNC router that makes CNC machining more accessible by inverting the traditional approach. Instead of moving a cutting tool within a fixed workspace, users guide the device around the workpiece directly while it automatically adjusts the cutting tool to stay on the programmed design path. This enables a significantly smaller device footprint while handling large-scale cuts.

## Architecture Overview

This is a **Teensy 4.1 Arduino firmware** for the Compass handheld CNC router. The system uses 4 PMW3360 optical flow mouse sensors for position tracking and a belt-driven coreXY gantry with TMC2209 stepper drivers for tool positioning.

**Core Components:**
- `src/main.cpp` - Main control loop: sensing → safety checks → path execution
- `src/config.h` - Hardware pins, motor parameters, timing constants
- `src/globals.h` - Global state objects (steppers, drivers, sensors, SD card)
- `src/types.h` - Core data structures (RouterPose, Path, Point, State enums)

## Key Architectural Patterns

### State Machine Design
The system operates through state transitions defined in `types.h`:
```cpp
enum State { IDLE, CUTTING, PAUSED, ZEROING, READY, MENU, ... };
```
Main loop sections are gated by `if (state != READY) return;` - respect this pattern.

### Modular Organization by Function
- `actuation/` - Motor control, PID, actuator coordination  
- `sensors/` - PMW3360 sensor reading and calibration
- `path/` - G-code parsing, path generation, execution logic
- `math/` - Coordinate transforms and geometric calculations
- `ui/` - Display, encoder, button handling
- `io/` - Serial/SD logging with binary packet protocols

### Hardware Abstraction
Physical constants are centralized in `config.h` with clear naming:
- Motor conversion factors: `ConvBelt`, `ConvLead` (steps/mm)
- Sensor geometry: `lx`, `ly` (sensor spacing in mm)
- Safety limits: `xRange`, `yRange`, `zRange` (work envelope)

## Build & Debug Workflow

**Build:** `pio run -e main-teensy41` (PlatformIO, not Arduino IDE)

**Debug Logging:** Set `outputMode = 1` in firmware, then:
```bash
pio device monitor > /logFiles/logFile_XX.txt
```
Log files can be analyzed with `debugViewer.ipynb` (binary packet format defined in `io/logging.h`).

## Critical Integration Points

### Sensor Calibration
Calibration coefficients stored in EEPROM, read at startup via `readEepromCalibration()`. Each sensor has `CalParams` with x/y/rotation corrections - never modify without understanding drift compensation.

### Real-time Control Loop
Main loop runs at ~1kHz with strict timing:
- **DMA sensor reading**: Hardware-accelerated via IntervalTimer (900μs) + DMA transfers
- Control updates: every 500μs (`dtControl` constant)
- **Non-blocking architecture**: Sensor SPI runs parallel to main loop via DMA
- Always use `micros()` for timing, never `delay()` in main loop
- Process sensor data only when `sensorDataReady` flag is set

### Safety System
Endstop checking via `checkEndstops()` halts motion immediately. All movement commands must respect work envelope limits defined in `config.h`.

### Path Execution
G-code files parsed into `Point` arrays (max `MAX_POINTS`). Path following uses look-ahead with feedrate control - see `path/path-execution.cpp` for trajectory generation.

## Development Conventions

- **Pin definitions:** All hardware pins defined in `config.h` with descriptive names
- **Units:** Consistent mm/mm/s throughout, with conversion factors for steps
- **Error handling:** Safety-first approach - return early from main loop on any fault
- **Global state:** Accessed via extern declarations in `globals.h`, defined in `globals.cpp`
- **Timing:** Use `elapsedMicros`/`elapsedMillis` types for interval timing