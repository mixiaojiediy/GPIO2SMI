# AGENTS.md

## Cursor Cloud specific instructions

### Project overview

This is an embedded firmware library implementing GPIO-based SMI (MDIO) protocol for reading/writing Ethernet PHY device registers. It contains two platform variants:

- **ESP32** (`esp32/`) — Arduino framework, C++
- **STM32** (`stm32/`) — STM32 HAL library (STM32F4xx), C

This code is designed for cross-compilation to microcontroller targets and **cannot run natively** on x86 Linux. Development verification is done via cross-compilation and static analysis only.

### Build & lint

Both variants use PlatformIO for cross-compilation and static analysis. PlatformIO project files (`platformio.ini`, `src/`, `include/`) are set up under each variant directory.

| Variant | Build | Lint (cppcheck) |
|---------|-------|-----------------|
| ESP32   | `cd esp32 && pio run` | `cd esp32 && pio check --skip-packages` |
| STM32   | `cd stm32 && pio run` | `cd stm32 && pio check --skip-packages` |

### Key caveats

- **STM32 external dependencies**: The STM32 variant depends on external headers (`bsp_SysTick.h`, `shell.h`, `board.h`, `common.h`) not present in the original repo. Minimal stubs are provided in `stm32/include/` to enable compilation.
- **No runtime testing possible**: Since this is embedded firmware targeting physical MCU hardware, there is no way to execute the code in the cloud VM. Verification is limited to successful cross-compilation and static analysis.
- **ESP32 narrowing warnings**: `soft_smi.cpp` produces `-Wnarrowing` warnings in `smi_set_reg()` — these are cosmetic and don't affect functionality.
- **PATH**: PlatformIO CLI is installed at `~/.local/bin`. Ensure `PATH` includes this directory (`export PATH="$HOME/.local/bin:$PATH"`).
