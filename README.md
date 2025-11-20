# STM32F411CEU6 Zephyr Recovery System

A Zephyr RTOS-based recovery and diagnostic system built with Yocto Project for the STM32F411CEU6 microcontroller ("Black Pill" board). This project creates a **single monolithic firmware binary** with an interactive shell interface for hardware testing and recovery operations.

---

## Project Status

**Successfully Building** - The project now builds successfully with Zephyr 3.6.0  
**Docker Environment** - Fully containerized build environment  
**Network Optimized** - Download queue limited to prevent WiFi issues  
**Ownership Fixed** - Automatic permission handling in Docker  

---

## Table of Contents

- [What is This Project?](#what-is-this-project)
- [Quick Start with Docker](#quick-start-with-docker)
- [Project Structure](#project-structure)
- [Building the Firmware](#building-the-firmware)
- [Flashing and Using](#flashing-and-using)
- [Available Shell Commands](#available-shell-commands)
- [Troubleshooting](#troubleshooting)
- [Technical Details](#technical-details)
- [Attribution](#attribution)

---

## What is This Project?

This project builds a **single Zephyr RTOS firmware binary** (~150KB) for the **STM32F411CEU6 microcontroller** (ARM Cortex-M4F @ 100MHz), using the **Yocto Project** build system. It provides:

- **Interactive shell interface** with recovery and diagnostic commands via UART
- **Hardware testing tools** for GPIO, I2C, SPI, and UART (implemented as shell commands)
- **System information and diagnostics**
- **Monolithic binary** - no filesystem, no separate packages, everything compiled into one .bin file
- **Real-time operation** with Zephyr RTOS kernel

### Key Features

**Zephyr RTOS 3.6.0** - Lightweight real-time operating system  
**Interactive Shell** - Command-line interface via UART console  
**GPIO Commands** - Pin control, monitoring, and testing  
**Interface Commands** - I2C, SPI, and UART testing  
**System Info** - Hardware details and Zephyr version  
**Tiny Footprint** - Single ~150KB binary, boots in <1 second  
**Docker Ready** - Complete containerized build environment  

### Target Hardware

- **Microcontroller:** STM32F411CEU6 (ARM Cortex-M4F @ 100MHz)
- **Board:** "Black Pill" or compatible STM32F411CEU6 development board
- **Flash Memory:** 512KB internal
- **RAM:** 128KB SRAM
- **Peripherals:** GPIO, USART, I2C, SPI, USB OTG FS, ADC, Timers
- **FPU:** Yes (Cortex-M4F with hardware floating point)

---

## Quick Start with Docker

### Prerequisites

- Docker and Docker Compose installed
- ~25GB free disk space for build
- Stable internet connection (for initial layer downloads)

### Step 1: Clone and Setup

```bash
# Clone the repository
git clone <your-repo-url>
cd yocto_project

# Set your user ID for proper file permissions
export UID=$(id -u)
export GID=$(id -g)
```

### Step 2: Build Docker Image

```bash
# Build the Docker image (first time only, takes ~10-15 minutes)
docker-compose build
```

### Step 3: Start Container

```bash
# Start the container
docker-compose up -d

# Enter the container
docker-compose exec yocto-builder bash
```

### Step 4: Build Firmware

```bash
# Inside the container
cd meta-mono
kas build kas/firmware.yaml

# Build takes ~30-60 minutes on first run
# Subsequent builds are much faster thanks to caching
```

### Step 5: Get Your Firmware

```bash
# Firmware will be in:
# meta-mono/build/tmp-newlib/deploy/images/stm32f411-dk/

# Main files:
# - zephyr-recovery.bin  (flash this to your STM32)
# - zephyr-recovery.elf  (for debugging)
# - zephyr-recovery.hex  (Intel HEX format)
```

### Step 6: Exit and Stop

```bash
# Exit container
exit

# Stop container (keeps data)
docker-compose stop

# Or remove container (keeps image)
docker-compose down
```

---

## Project Structure

```
yocto_project/
├── Dockerfile                  # Docker build environment
├── docker-compose.yml          # Docker orchestration
├── README.md                   # This file
│
└── meta-mono/                  # Custom Yocto layer
    ├── conf/                   # Layer configuration
    │   ├── layer.conf          # Layer definition
    │   ├── distro/
    │   │   └── recovery.conf   # Distribution config (Zephyr-based)
    │   └── machine/
    │       └── stm32f411-dk.conf  # Hardware config
    │
    ├── kas/
    │   └── firmware.yaml       # KAS build configuration (MAIN CONFIG)
    │
    ├── recipes-kernel/
    │   └── zephyr-recovery/    # Main application recipe
    │       ├── zephyr-recovery_1.0.bb  # Build recipe
    │       └── files/
    │           ├── src/main.c          # Application code
    │           ├── prj.conf            # Zephyr config
    │           ├── CMakeLists.txt      # Build config
    │           └── app.overlay         # Device tree overlay
    │
    ├── build/                  # Build output (created during build)
    │   ├── downloads/          # Source tarballs (2.5GB)
    │   ├── sstate-cache/       # Build cache
    │   ├── cache/              # BitBake cache
    │   └── tmp-newlib/         # Build artifacts (17GB)
    │
    └── sources/                # Downloaded Yocto layers
        ├── bitbake/
        ├── openembedded-core/
        ├── meta-zephyr/
        ├── meta-arm/
        └── meta-openembedded/
```

---

## Building the Firmware

### Using Docker (Recommended)

```bash
# Start container
docker-compose up -d
docker-compose exec yocto-builder bash

# Build
cd meta-mono
kas build kas/firmware.yaml
```

### Without Docker (Advanced)

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install gawk wget git diffstat unzip texinfo gcc build-essential \
    chrpath socat cpio python3 python3-pip python3-pexpect xz-utils \
    debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa \
    libsdl1.2-dev xterm python3-subunit mesa-common-dev zstd liblz4-tool \
    gcc-arm-none-eabi ninja-build

# Install KAS
pip3 install kas

# Build
cd meta-mono
kas build kas/firmware.yaml
```

### Build Configuration

The build is configured in `meta-mono/kas/firmware.yaml`:

```yaml
machine: stm32f411-dk          # Target hardware
distro: recovery               # Distribution type
target:
  - zephyr-recovery            # What to build

local_conf_header:
  fetch_threads: |
    BB_NUMBER_FETCH_THREADS = "4"  # Limit downloads to prevent network issues
```

### Build Times

| Build Type | Time | Notes |
|------------|------|-------|
| **First Build** | 30-60 min | Downloads all sources |
| **Clean Rebuild** | 15-30 min | Uses cached downloads |
| **Incremental** | 2-5 min | Only rebuilds changes |

### Build Output

```bash
# Output location:
meta-mono/build/tmp-newlib/deploy/images/stm32f411-dk/

# Files:
zephyr-recovery.bin    # Raw binary (flash this)
zephyr-recovery.elf    # ELF with debug symbols
zephyr-recovery.hex    # Intel HEX format
```

---

## Flashing and Using

### Flashing to STM32F411

#### Using st-flash (Recommended)

```bash
# Install st-flash
sudo apt-get install stlink-tools

# Flash the binary
st-flash write meta-mono/build/tmp-newlib/deploy/images/stm32f411-dk/zephyr-recovery.bin 0x08000000
```

#### Using OpenOCD

```bash
# Install OpenOCD
sudo apt-get install openocd

# Flash using OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
    -c "program meta-mono/build/tmp-newlib/deploy/images/stm32f411-dk/zephyr-recovery.elf verify reset exit"
```

### Connecting to Serial Console

```bash
# Using screen
screen /dev/ttyACM0 115200

# Or using minicom
minicom -D /dev/ttyACM0 -b 115200

# Or using picocom
picocom -b 115200 /dev/ttyACM0
```

**Serial Settings:** 115200 baud, 8N1 (8 data bits, no parity, 1 stop bit)

---

## Available Shell Commands

Once connected to the serial console, you can use these commands:

### System Commands

```bash
# Display system information
system info

# Example output:
# === STM32F411CEU6 System Information ===
# Device: STM32F411CEU6 (Black Pill)
# CPU: ARM Cortex-M4 @ 100MHz
# Flash: 512KB
# RAM: 128KB
# Zephyr Version: 3.6.0
```

### GPIO Commands

```bash
# Test GPIO functionality
gpio test

# Set GPIO pin
gpio set <port> <pin> <value>
# Example: gpio set C 13 1    # Turn on LED on PC13

# Monitor GPIO state
gpio monitor <port> <pin>
# Example: gpio monitor C 15  # Monitor button on PC15
```

### I2C Commands

```bash
# Scan I2C bus
i2c scan <bus>
# Example: i2c scan 1

# Read from I2C device
i2c read <bus> <address> <register> <length>

# Write to I2C device
i2c write <bus> <address> <register> <data...>
```

### SPI Commands

```bash
# Test SPI communication
spi test <bus> <data...>
# Example: spi test 1 0xAA 0xBB 0xCC
```

### UART Commands

```bash
# Test UART communication
uart test <port> <mode>
# Example: uart test 2 send "Hello"
```

---

## Troubleshooting

### Docker Issues

#### Permission/Ownership Problems

**Problem:** Files created in container have wrong ownership

**Solution:** The Docker setup now automatically fixes this! The entrypoint script runs:
```bash
chown -R yoctouser:yoctouser /workspace/meta-mono
```

If you still have issues:
```bash
# Rebuild with your UID/GID
export UID=$(id -u)
export GID=$(id -g)
docker-compose build --no-cache
```

#### Container Won't Start

```bash
# Check logs
docker-compose logs

# Restart container
docker-compose restart

# Rebuild from scratch
docker-compose down
docker-compose build --no-cache
docker-compose up -d
```

### Build Issues

#### Network/Download Errors

**Problem:** WiFi disconnects during build, fetch errors

**Solution:** Already configured! `BB_NUMBER_FETCH_THREADS = "4"` limits parallel downloads.

If still having issues:
```bash
# Edit meta-mono/kas/firmware.yaml
# Change BB_NUMBER_FETCH_THREADS to "2" or "1"
```

#### Python Module Errors

**Problem:** `ModuleNotFoundError: No module named 'pykwalify'`

**Solution:** Already fixed! The recipe includes:
```
DEPENDS += "python3-pyelftools-native python3-pyyaml-native python3-pykwalify-native"
```

#### Compilation Errors

**Problem:** `undefined reference to KERNEL_VERSION_STRING`

**Solution:** Already fixed! Using `<version.h>` include and `KERNEL_VERSION_STRING` macro.

#### Out of Disk Space

```bash
# Check space
df -h

# Clean build artifacts (keeps downloads)
cd meta-mono
rm -rf build/tmp-newlib

# Clean everything (will re-download)
rm -rf build/tmp-newlib build/downloads build/sstate-cache
```

### Runtime Issues

#### No Serial Output

1. Check USB connection
2. Verify correct serial port: `ls /dev/ttyACM*` or `ls /dev/ttyUSB*`
3. Check baud rate: 115200
4. Try different terminal program

#### Commands Don't Work

1. Verify firmware flashed correctly
2. Check Zephyr shell is enabled in `prj.conf`
3. Try pressing Enter to get shell prompt
4. Type `help` to see available commands

---

## Technical Details

### Software Stack

| Component | Version | Purpose |
|-----------|---------|---------|
| **Yocto Project** | Scarthgap | Build system |
| **Zephyr RTOS** | 3.6.0 | Operating system |
| **BitBake** | 2.8 | Build engine |
| **KAS** | Latest | Build orchestration |
| **CMake** | 3.17.2 | Zephyr build system |
| **Zephyr SDK** | 0.16.8 | ARM toolchain |

### Hardware Specifications

| Component | Specification |
|-----------|--------------|
| **MCU** | STM32F411CEU6 |
| **Core** | ARM Cortex-M4F @ 100MHz |
| **Flash** | 512KB internal |
| **RAM** | 128KB SRAM |
| **FPU** | Yes (hardware floating point) |
| **Peripherals** | GPIO, USART, I2C, SPI, USB, ADC, Timers |

### Build System Details

| Metric | Value |
|--------|-------|
| **Build Directory Size** | ~20GB |
| **Downloads** | ~2.5GB |
| **Build Artifacts** | ~17GB |
| **Final Binary Size** | ~150KB |
| **Boot Time** | <1 second |

### Network Optimization

To prevent WiFi disconnections during builds:

```yaml
# In kas/firmware.yaml
local_conf_header:
  fetch_threads: |
    BB_NUMBER_FETCH_THREADS = "4"
```

This limits parallel downloads from default 8+ to 4, preventing network overload.

---

## File Necessity Guide

### KEEP - Essential

| Path | Size | Purpose |
|------|------|---------|
| `meta-mono/conf/` | 24KB | Layer configuration |
| `meta-mono/kas/` | Small | Build configuration |
| `meta-mono/recipes-*/` | Small | Build recipes |
| `build/downloads/` | 2.5GB | Source tarballs (avoids re-download) |
| `build/cache/` | 1.4MB | BitBake cache (speeds up parsing) |

### CAN DELETE - But Requires Rebuild

| Path | Size | Impact |
|------|------|--------|
| `build/tmp-newlib/` | 17GB | All build artifacts - full rebuild needed |
| `build/sstate-cache/` | Varies | Shared state cache - slower rebuilds |

### SAFE TO DELETE - Regenerated

| Path | Purpose |
|------|---------|
| `*.log` files | Build logs |
| `bitbake-cookerdaemon.log` | BitBake daemon log |
| `tmp/work/*/temp/` | Task logs |

---

## Learning Resources

### STM32 Documentation
- [STM32F411 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00119316.pdf)
- [STM32F411 Datasheet](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf)

### Zephyr RTOS
- [Zephyr Documentation](https://docs.zephyrproject.org/)
- [Zephyr Getting Started](https://docs.zephyrproject.org/latest/getting_started/index.html)
- [Zephyr Shell Guide](https://docs.zephyrproject.org/latest/services/shell/index.html)

### Yocto Project
- [Yocto Project Documentation](https://docs.yoctoproject.org/)
- [BitBake User Manual](https://docs.yoctoproject.org/bitbake/)
- [KAS Documentation](https://kas.readthedocs.io/)

---

## Attribution

### Based on ARM Cortex-A72 Tutorial

This project was originally based on a Yocto Project tutorial for **ARM Cortex-A72** (NXP LS1046A Gateway).

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

## Errors

0) If ur having issues fetchin / downloading files dont panic! YOCTO and zephyr are very resourcefull in terms of internet connection so it may get u out of ur wifi for a moment but if it does, simply rebuild the project with kas build kas/firmware.yaml and u'll be good.

1) kas warning error -> we were having cmsis warining that cause entire build to collapse. We had to provide targets for correct targets for cmsis to work.  

2) cmake mismatch newer versions of zephyr require cmake >= 3.20.5 -> we were having issues cause it could build dummy file -> it was cause bu gcc mismatch between yocto and zephyr yocto tries t use flag which isnt introduced in current zehpyr sdk toolchain -> we had to update zephyr-recovery_1.0.bb recipe to override the DEBUG_PREFIX_MAP variable to use only the older flags
(-fmacro-prefix-map and -fdebug-prefix-map) that GCC 12 supports (from Zephyr).  

3) missing modules for cmake -> added missing deps just so (they were a lot of these) all added 2: zephyr-recovery_1.0.bb  

---

## License

All custom tools and modifications are provided under MIT License.  
Zephyr RTOS is provided under Apache-2.0 license.  
Yocto Project components under their respective licenses.

---

## Contributing

Contributions are welcome! Areas for improvement:

- Additional shell commands
- More hardware peripheral support
- Port to other STM32 families
- Documentation improvements
- Bug fixes and optimizations

---

## Support

For issues or questions:
1. Check this README and troubleshooting section
2. Review Zephyr RTOS documentation
3. Consult Yocto Project resources
4. Open an issue on the repository

---

**Project Version:** 2.0 (STM32F411 + Zephyr 3.6.0)  
**Last Updated:** January 2025  
**Build Status:** Successfully Building  
**Target Platform:** STM32F411 Development Kit  

---

*Built with ❤️ using Yocto Project, Zephyr RTOS, and Docker*  
*MIT License © 2025*
