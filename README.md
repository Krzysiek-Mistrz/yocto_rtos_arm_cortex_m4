# STM32F411CEU6 Zephyr Recovery System

A Zephyr RTOS-based recovery and diagnostic system built with Yocto Project for the STM32F411CEU6 microcontroller ("Black Pill" board). This project creates a **single monolithic firmware binary** with an interactive shell interface for hardware testing and recovery operations.

---

## Table of Contents

- [What is This Project?](#what-is-this-project)
- [Project Structure](#project-structure)
- [How to Use This Project](#how-to-use-this-project)
- [Available Tools](#available-tools)
- [Key Yocto Concepts](#key-yocto-concepts)
- [How It All Works Together](#how-it-all-works-together)
- [Attribution](#attribution)

---

## What is This Project?

This project builds a **single Zephyr RTOS firmware binary** (~150KB) for the **STM32F411CEU6 microcontroller** (ARM Cortex-M4F @ 100MHz), using the **Yocto Project** build system. It provides:

- **Interactive shell interface** with recovery and diagnostic commands via UART
- **Hardware testing tools** for GPIO, I2C, SPI, and UART (implemented as shell commands)
- **Flash programming and memory testing** capabilities
- **Monolithic binary** - no filesystem, no separate packages, everything compiled into one .bin file
- **Real-time operation** with Zephyr RTOS kernel

### Key Features

**Zephyr RTOS** - Lightweight real-time operating system for microcontrollers  
**Interactive Shell** - Command-line interface via UART console  
**GPIO Commands** - Shell commands for GPIO control, monitoring, and testing  
**Interface Commands** - I2C, SPI, and UART testing via shell  
**Recovery Commands** - Flash programming, memory testing, and system info  
**Tiny Footprint** - Single ~150KB binary, boots in <1 second  
**No Filesystem** - Monolithic binary, all code compiled in  

### Target Hardware

- **Microcontroller:** STM32F411CEU6 (ARM Cortex-M4F @ 100MHz)
- **Board:** "Black Pill" or compatible STM32F411CEU6 development board
- **Flash Memory:** 512KB internal
- **RAM:** 128KB SRAM
- **Peripherals:** GPIO, USART, I2C, SPI, USB OTG FS, ADC, Timers
- **FPU:** Yes (Cortex-M4F with hardware floating point)

---

## Project Structure

### meta-mono/ - Your Custom Yocto Layer

The "meta-" prefix in Yocto means it's a layer (a collection of recipes and configurations). "mono" is just the name. Think of a layer as a plugin or module that adds specific functionality to Yocto.

---

### Inside meta-mono/:

#### 1. conf/ - Configuration Files (The Settings)

This is where you configure HOW things should be built.

- **`layer.conf`** - Tells Yocto "Hey, I'm a layer called meta-mono, here's where my recipes are!"
- **`site.conf`** - Build performance settings (like using 24 CPU cores, where to cache downloads)
- **`distro/recovery.conf`** - Defines your embedded distribution settings:
  - Name: "Mono Recovery Linux"
  - Uses newlib (lightweight C library for embedded systems)
  - Zephyr RTOS instead of full Linux
  - No systemd, no GUI - super minimal!
  - Features: GPIO, I2C, SPI, USB, UART support

- **`machine/stm32f411-dk.conf`** - Hardware-specific settings:
  - Target device: STM32F411 Development Kit
  - Processor: ARM Cortex-M4 with FPU
  - SOC Family: STM32F4 (STM32F411)
  - Kernel type: Zephyr RTOS
  - Device tree, serial console settings
  - Image formats: bin, elf, hex (for embedded flashing)

---

#### 2. kas/ - KAS Build Configuration

- **`firmware.yaml`** - This is a **KAS** file (KAS is a tool that makes Yocto easier to use)
  - It tells KAS: "Download these Yocto layers from Git, use this machine config, use this distro config"
  - Basically the **master blueprint** that pulls everything together
  - Configured for: `stm32f411-dk` machine
  - Includes layers: BitBake, OpenEmbedded-Core, meta-zephyr, meta-arm

- **`kas_shell_history`** - Just command history from when someone used KAS shell

---

#### 3. recipes-kernel/ - Kernel/RTOS Recipes

##### zephyr-recovery/ - Main Zephyr Recovery Application

- **`zephyr-recovery_1.0.bb`** - Recipe to build the Zephyr recovery application
- **`files/`** - Application source code and configuration:
  - **`src/main.c`** - Main application with all shell commands implemented
  - **`prj.conf`** - Zephyr kernel configuration (enables shell, GPIO, I2C, SPI, UART drivers)
  - **`CMakeLists.txt`** - CMake build configuration for Zephyr

**This is the main recipe that builds the entire recovery system as a single binary.**

---

#### 4. recipes-bsp/ - Board Support Package

- **`stm32-hal/`** - STM32F4 HAL (if present, for reference)
  - **Note:** Zephyr uses its own device drivers, not STM32 HAL
  - This directory may contain HAL code for reference purposes only

---

#### 5. recipes-support/ - Reference Code (NOT USED IN BUILD)

**Important:** These directories contain **Linux userspace C code** that is **NOT compiled** into the Zephyr binary. They are kept as **reference material** for implementing the actual Zephyr shell commands.

##### gpio-tools/ - GPIO Reference Code

- **`files/`** - Linux userspace GPIO code (reference only):
  - **`gpio-test.c`** - Example GPIO toggle code
  - **`gpio-monitor.c`** - Example GPIO monitoring
  - **`gpio-set.c`** - Example GPIO control
  - **`Makefile`** - Not used

##### interface-tools/ - Communication Interface Reference Code

- **`files/`** - Linux userspace communication code (reference only):
  - **`i2c-scan.c`** - Example I2C bus scanning
  - **`i2c-read.c`** - Example I2C read operations
  - **`i2c-write.c`** - Example I2C write operations
  - **`spi-test.c`** - Example SPI communication
  - **`uart-test.c`** - Example UART communication
  - **`Makefile`** - Not used

##### recovery-tools/ - Recovery Tools Reference Code

- **`files/`** - Linux userspace recovery code (reference only):
  - **`flash-program.c`** - Example flash programming logic
  - **`memory-test.c`** - Example memory testing patterns
  - **`system-info.c`** - Example system information display
  - **`Makefile`** - Not used

**These files can be used as reference when implementing the actual functionality in `recipes-kernel/zephyr-recovery/files/src/main.c` using Zephyr APIs.**

---

#### 7. build/ - Build Output Directory

Where Yocto puts all the compiled files, temporary files, and final images when you build. Usually empty until you run a build.

**Output location:** `build/tmp/deploy/images/stm32f411-dk/`

---

#### 8. sources/ - Downloaded Source Code

Where Yocto downloads external layers and source code (like OpenEmbedded-Core, BitBake, meta-zephyr, meta-arm). The firmware.yaml tells it to put things here.

---

## How to Use This Project

### Prerequisites

Before building, you need to install KAS and Yocto dependencies:

```bash
# Install KAS build tool
pip3 install kas

# Install Yocto Project dependencies (Ubuntu/Debian)
sudo apt-get install gawk wget git diffstat unzip texinfo gcc build-essential \
    chrpath socat cpio python3 python3-pip python3-pexpect xz-utils \
    debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa \
    libsdl1.2-dev pylint3 xterm python3-subunit mesa-common-dev zstd liblz4-tool

# Install ARM toolchain for Cortex-M
sudo apt-get install gcc-arm-none-eabi
```

---

### Building the Recovery Image

```bash
# Navigate to the meta-mono layer
cd /path/to/yocto_project/meta-mono

# Build the image using KAS
kas build kas/firmware.yaml

# This will:
# 1. Download all required Yocto layers (BitBake, OpenEmbedded-Core, meta-zephyr, meta-arm)
# 2. Configure the build for STM32F411
# 3. Compile Zephyr RTOS
# 4. Build STM32 HAL library
# 5. Compile all custom tools (GPIO, interface, recovery)
# 6. Create the final recovery image

# Build output will be in:
# build/tmp/deploy/images/stm32f411-dk/
```

**Output files:**
- `recovery-image-stm32f411-dk.bin` - Raw binary for flashing
- `recovery-image-stm32f411-dk.elf` - ELF file with debug symbols
- `recovery-image-stm32f411-dk.hex` - Intel HEX format

---

### Flashing to STM32F411

#### Using st-flash (STM32 Programmer)

```bash
# Install st-flash
sudo apt-get install stlink-tools

# Flash the binary to STM32F411
st-flash write build/tmp/deploy/images/stm32f411-dk/recovery-image-stm32f411-dk.bin 0x08000000
```

#### Using OpenOCD

```bash
# Install OpenOCD
sudo apt-get install openocd

# Flash using OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
    -c "program build/tmp/deploy/images/stm32f411-dk/recovery-image-stm32f411-dk.elf verify reset exit"
```

---

### Connecting to the Device

```bash
# Connect to serial console (USART2 via ST-Link)
# Baud rate: 115200, 8N1
screen /dev/ttyACM0 115200

# Or using minicom
minicom -D /dev/ttyACM0 -b 115200
```

---

## Available Tools

Once the system is running on your STM32F411, you can use these command-line tools:

### GPIO Tools

```bash
# Toggle LED on PC13 (10 times, 500ms interval)
gpio-test C 13 10

# Monitor button state on PC15 (check every 100ms)
gpio-monitor C 15 100

# Set GPIO pin HIGH or LOW
gpio-set C 13 1      # Set PC13 HIGH (LED ON)
gpio-set C 13 0      # Set PC13 LOW (LED OFF)

# Read GPIO pin state
gpio-set B 7 read    # Read current state of PB7
```

---

### I2C Tools

```bash
# Scan I2C1 bus for connected devices
i2c-scan 1

# Read 16 bytes from I2C EEPROM at address 0x50, starting at register 0x00
i2c-read 1 0x50 0x00 16

# Write 3 bytes to I2C device at address 0x50, register 0x00
i2c-write 1 0x50 0x00 0xAA 0xBB 0xCC
```

---

### SPI Tools

```bash
# Send 4 bytes on SPI1 and display received data
spi-test 1 0xAA 0xBB 0xCC 0xDD
```

---

### UART Tools

```bash
# Send text on USART2
uart-test 2 send "Hello STM32!"

# Receive data on USART1
uart-test 1 receive

# Loopback mode (echo) on USART6
uart-test 6 loopback
```

---

### Recovery Tools

```bash
# Display comprehensive system information
system-info

# Quick RAM test (16KB, basic patterns)
memory-test quick

# Full RAM test (124KB, 8 patterns)
memory-test full

# Walking bit test (4KB)
memory-test walk

# Display memory layout
memory-test info

# Display flash information and sector map
flash-program info

# Read 256 bytes from flash starting at 0x08000000
flash-program read 0x08000000 256

# Erase flash sector 5 (CAUTION!)
flash-program erase 5

# Write 32-bit word to flash address (CAUTION!)
flash-program write 0x08010000 0xDEADBEEF
```

**WARNING:** Flash programming operations can brick your device if used incorrectly. Always verify addresses and understand the flash layout before erasing or writing!

---

## Key Yocto Concepts

- **`.bb` files** = "BitBake recipes" - instructions on how to build something (like a Makefile)
- **`.conf` files** = Configuration files - settings and variables
- **`.yaml` files** = KAS configuration - orchestrates the whole build
- **`meta-*`** = Layers - modular components you can stack together
- **`recipes-*`** = Folders containing recipes organized by category (core, kernel, bsp, support)
- **Machine** = Hardware platform definition (e.g., stm32f411-dk)
- **Distro** = Distribution configuration (e.g., recovery)
- **Image** = Final output that gets flashed to the device

---

## How It All Works Together

1. **KAS** reads `firmware.yaml`
2. Downloads **BitBake** (the build engine) and **OpenEmbedded-Core** (base Yocto layer) into `sources/`
3. Downloads **meta-zephyr** (Zephyr RTOS support) and **meta-arm** (ARM toolchain)
4. Uses **machine config** (`stm32f411-dk.conf`) to know the hardware specifications
5. Uses **distro config** (`recovery.conf`) to know what kind of embedded system to build
6. Builds the **STM32 HAL** library using `stm32f4-hal_1.28.0.bb` recipe
7. Builds the **Zephyr kernel** using `zephyr-kernel_3.5.bb` recipe with device tree and defconfig
8. Builds all **custom tools** (GPIO, interface, recovery) using their respective recipes
9. Builds the **recovery image** using `recovery-image.bb` recipe, bundling everything together
10. Outputs bootable binary, ELF, and HEX files for your STM32F411 device

---

## In Summary

You're building a **minimal recovery and development system** for an **ARM Cortex-M4 microcontroller**. It's like creating a bootable USB recovery drive, but for embedded hardware. The system is super tiny (fits in <1MB) so it boots fast (<1 second) and provides essential tools for:

- **Hardware testing** (GPIO, I2C, SPI, UART)
- **System diagnostics** (memory tests, system info)
- **Recovery operations** (flash programming, memory validation)
- **Development and debugging** (command-line utilities with STM32 HAL)

Unlike traditional Linux systems, this uses **Zephyr RTOS** - a lightweight real-time operating system designed specifically for microcontrollers with limited resources.

---

## Technical Specifications

| Component | Specification |
|-----------|--------------|
| **Target MCU** | STM32F411 (ARM Cortex-M4 @ 100MHz) |
| **Flash Memory** | 512KB internal |
| **RAM** | 128KB SRAM |
| **Operating System** | Zephyr RTOS 3.5.0 |
| **HAL Version** | STM32CubeF4 v1.28.0 |
| **C Library** | newlib (embedded) |
| **Image Size** | <1MB |
| **Boot Time** | <1 second |
| **Yocto Version** | Walnascar (compatible) |
| **Build System** | KAS + BitBake |

---

## Learning Resources

### STM32 Documentation
- [STM32F411 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00119316.pdf)
- [STM32F411 Datasheet](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf)
- [STM32 HAL Documentation](https://www.st.com/resource/en/user_manual/dm00105879.pdf)

### Zephyr RTOS
- [Zephyr Documentation](https://docs.zephyrproject.org/)
- [Zephyr Getting Started](https://docs.zephyrproject.org/latest/getting_started/index.html)

### Yocto Project
- [Yocto Project Documentation](https://docs.yoctoproject.org/)
- [BitBake User Manual](https://docs.yoctoproject.org/bitbake/)
- [KAS Documentation](https://kas.readthedocs.io/)

---

## Troubleshooting

### Build Issues

**Problem:** Missing meta-zephyr layer  
**Solution:**
```bash
cd sources
git clone https://github.com/zephyrproject-rtos/meta-zephyr.git
```

**Problem:** ARM toolchain not found  
**Solution:**
```bash
sudo apt-get install gcc-arm-none-eabi
```

**Problem:** KAS command not found  
**Solution:**
```bash
pip3 install --user kas
# Add ~/.local/bin to PATH if needed
export PATH=$PATH:~/.local/bin
```

### Runtime Issues

**Problem:** GPIO tools don't work  
**Solution:** Check that GPIO clocks are enabled in device tree and HAL is properly initialized

**Problem:** I2C scan finds no devices  
**Solution:**
- Verify I2C pull-up resistors (4.7kΩ typical)
- Check device tree pin configuration
- Verify I2C bus number matches hardware

**Problem:** Flash programming fails  
**Solution:**
- Ensure flash is not write-protected
- Check address is within valid range (0x08000000-0x0807FFFF)
- Verify sector is erased before writing

---

## License

All custom tools and modifications are provided under MIT License.  
STM32 HAL is provided under BSD-3-Clause by STMicroelectronics.  
Zephyr RTOS is provided under Apache-2.0 license.

---

## Attribution

### Based on ARM Cortex-A72 Tutorial

This project was originally based on a Yocto Project tutorial for **ARM Cortex-A72** (NXP LS1046A Gateway) from a YouTube tutorial. The original project configuration and structure can be found at:

**Original Tutorial Repository:**  
https://gist.github.com/tomazzaman/c66ef5870839c1b8cd53e9931bd11ba7

### Transformation

The project has been completely transformed from:
- **ARM Cortex-A72** (application processor, 4-core @ 1.8GHz) → **ARM Cortex-M4** (microcontroller, 1-core @ 100MHz)
- **Full Linux with BusyBox** → **Zephyr RTOS**
- **2GB DDR4 RAM** → **128KB SRAM**
- **External QSPI Flash** → **512KB Internal Flash**
- **~50MB+ image size** → **<1MB image size**
- **~10 second boot time** → **<1 second boot time**

---

## Contributing

Contributions are welcome! If you'd like to add new tools, improve existing ones, or port to other STM32 families, please feel free to submit pull requests.

---

## Support

For issues or questions:
1. Check this README and the transformation guide
2. Review STM32 HAL documentation
3. Consult Zephyr RTOS documentation
4. Check Yocto Project resources

---

**Project Version:** 2.0 (STM32F411)  
**Last Updated:** 2025  
**Target Platform:** STM32F411 Development Kit  

---

*Built with ❤️ using Yocto Project, Zephyr RTOS, and STM32 HAL*  
MIT @ Krzychu 2025