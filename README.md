# STM32F411 Zephyr & i.MX6ULL Linux Bridge System

A unified embedded system project combining a **Zephyr RTOS** recovery firmware for the STM32F411CEU6 ("Black Pill") and a custom **Yocto Linux** image for the i.MX6ULL. The two systems interact via a dedicated UART bridge, allowing the Linux application processor to manage, diagnose, and recover the real-time MCU.

The project is fully containerized using Docker, ensuring reproducible builds for both the firmware and the Linux image.

---

## Project Status

**Current Date:** December 20, 2025  
**Version:** 2.1  

- **Build Status:** ✅ Stable & Reproducible
- **Zephyr Version:** 3.6.0 (via `meta-zephyr` Scarthgap)
- **Linux Version:** Yocto "Scarthgap" (5.0)
- **Container:** Docker-based build environment with caching and network optimization

---

## Table of Contents

- [System Architecture](#system-architecture)
- [Quick Start](#quick-start)
- [Project Structure](#project-structure)
- [Building the Firmware (STM32)](#building-the-firmware-stm32)
- [Building the Linux Image (i.MX6ULL)](#building-the-linux-image-imx6ull)
- [Software Stack Details](#software-stack-details)
- [Flashing & Hardware](#flashing--hardware)
- [UART Bridge Protocol](#uart-bridge-protocol)
- [Troubleshooting](#troubleshooting)
- [License](#license)

---

## System Architecture

```
┌──────────────────────────────┐              UART               ┌──────────────────────────────┐
│   i.MX6ULL (Cortex-A7)       │◄───────────────────────────────►│   STM32F411 (Cortex-M4F)     │
│                              │          115200 baud            │                              │
│  [ Linux User Space ]        │                                 │  [ Zephyr RTOS ]             │
│   - uart-bridge daemon       │                                 │   - Command Shell            │
│   - Python/Shell tools       │                                 │   - GPIO/I2C/SPI Drivers     │
│   - SSH / Networking         │                                 │   - Watchdog / Recovery      │
└──────────────────────────────┘                                 └──────────────────────────────┘
```

The system is designed as a "Split-Brain" architecture:
1.  **The Brain (Linux):** Handles network, storage, complex logic, and remote access.
2.  **The Hands (Zephyr):** Handles real-time I/O, hardware safety, and board bring-up.

---

## Quick Start

### Prerequisites
- Docker & Docker Compose
- Git
- ~30GB Disk Space

### Setup Environment
```bash
# 1. Clone the repository and go into prj
cd yocto_project

# 2. Start the builder container
docker-compose up -d

# 3. Enter the container
docker-compose exec yocto-builder bash
```

---

## Project Structure

```
yocto_project/
├── Dockerfile                     # Multi-stage build environment
├── docker-compose.yml             # Container orchestration
├── meta-mono/                     # Custom Yocto Layer
│   ├── conf/                      # Layer & Distro configs
│   ├── kas/                       # Build definitions (The "Makefiles" of this project)
│   │   ├── firmware.yaml          # -> Builds STM32 Zephyr Firmware
│   │   └── imx6ull-image.yaml     # -> Builds i.MX6ULL Linux Image
│   ├── recipes-connectivity/
│   │   └── uart-bridge/           # Custom C daemon for UART communication
│   ├── recipes-core/
│   │   └── images/                # Linux Image definitions
│   └── recipes-kernel/
│       └── zephyr-recovery/       # Zephyr App source & board overlay
└── ...
```

---

## Building the Firmware (STM32)

Inside the Docker container:

```bash
cd meta-mono
kas build kas/firmware.yaml
```

**Artifacts Location:**  
`meta-mono/build/tmp-newlib/deploy/images/stm32f411-dk/`

**Files:**
- `zephyr-recovery.bin` (Raw binary for st-flash)
- `zephyr-recovery.elf` (For GDB/OpenOCD)

**Build Specs:**
- **Board:** `blackpill_f411ce`
- **SDK:** Zephyr SDK 0.16.8
- **Size:** ~150KB

---

## Building the Linux Image (i.MX6ULL)

Inside the Docker container:

```bash
cd meta-mono
kas build kas/imx6ull-image.yaml
```

**Artifacts Location:**  
`meta-mono/build/tmp/deploy/images/imx6ull-ebyte/`

**Files:**
- `imx6ull-uart-bridge-image-imx6ull-ebyte.wic.gz` (SD Card Image)
- `zImage` (Kernel)
- `imx6ull-ebyte-emmc.dtb` (Device Tree)

**Build Specs:**
- **Distro:** `imx6ull-linux`
- **Machine:** `imx6ull-ebyte`

---

## Software Stack Details

### Linux Image (`imx6ull-uart-bridge-image`)
The Linux image is minimal but feature-rich for development:

*   **Core:** Systemd, Coreutils, Procps
*   **Networking:** OpenSSH Server (sftp enabled), iproute2
*   **Editors:** Vim, Nano
*   **Monitoring:** Htop, systemd-analyze
*   **Hardware Tools:**
    *   `i2c-tools` (i2cdetect, i2cget, etc.)
    *   `spitools`
    *   `uart-bridge` (Custom daemon)
*   **Debugging:** GDB, Strace, Tcpdump

### Zephyr Firmware (`zephyr-recovery`)
A monolithic RTOS binary providing:

*   **Shell:** Interactive UART console
*   **Drivers:** GPIO, UART, I2C, SPI
*   **Subsystems:** Logging, Shell, Device Tree
*   **Custom App:** Protocol handler for commands from Linux

---

## Flashing & Hardware

### STM32F411 (Black Pill)
Flash using `st-flash` (outside docker):
```bash
st-flash write meta-mono/build/tmp-newlib/deploy/images/stm32f411-dk/zephyr-recovery.bin 0x08000000
```

### i.MX6ULL
Flash to SD card:
```bash
zcat meta-mono/build/tmp/deploy/images/imx6ull-ebyte/imx6ull-uart-bridge-image-imx6ull-ebyte.wic.gz | sudo dd of=/dev/sdX bs=4M status=progress && sync
```

### Wiring (UART Bridge)
| i.MX6ULL (UART2) | STM32F411 (USART1) |
|------------------|--------------------|
| TX (Pin X)       | PA3 (RX)           |
| RX (Pin Y)       | PA2 (TX)           |
| GND              | GND                |

*Note: Both run at 3.3V logic.*

---

## UART Bridge Protocol

Communication happens at **115200 baud 8N1**. The `uart-bridge` daemon on Linux exposes a socket at `/var/run/uart-bridge.sock` for local tools to send commands to the STM32.

**Command Format:** `CMD:ARG1,ARG2`  
**Example:** `GPIO_SET:C,13,1` (Sets PC13 High)

**Testing from Linux Terminal:**
```bash
# Send a ping to STM32
echo "PING" | socat - UNIX-CONNECT:/var/run/uart-bridge.sock

# Toggle LED
echo "GPIO_SET:C,13,0" | socat - UNIX-CONNECT:/var/run/uart-bridge.sock
```

---

## Troubleshooting

1.  **Build Fails (Network):**
    The configuration is tuned for stability (`BB_NUMBER_FETCH_THREADS = "4"`). If issues persist, check your proxy settings or reduce threads in `kas/firmware.yaml`.

2.  **Permissions:**
    The Docker container automatically fixes permissions. If you see "Permission denied" on generated files, run `docker-compose restart`.

3.  **UART Silence:**
    - Verify RX/TX are crossed (TX->RX, RX->TX).
    - Check if `uart-bridge` service is running: `systemctl status uart-bridge`.
    - Stop the service (`systemctl stop uart-bridge`) to access the raw serial port (`/dev/ttymxc1`) via `minicom`.

---

## License

All custom tools and modifications are provided under MIT License.
Zephyr RTOS is provided under Apache-2.0 license.
Yocto Project components under their respective licenses.

---

*Built with ❤️ using Yocto Project, Zephyr RTOS, and Docker*
*MIT License © 2025*