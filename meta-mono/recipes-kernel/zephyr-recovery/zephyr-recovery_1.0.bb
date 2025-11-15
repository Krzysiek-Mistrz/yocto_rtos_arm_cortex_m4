SUMMARY = "STM32F411CEU6 Recovery System - Zephyr RTOS Application"
DESCRIPTION = "Zephyr-based recovery and diagnostic system for STM32F411CEU6 (Black Pill) with GPIO, I2C, SPI, and UART tools"

# Use the zephyr-sample base configuration
require recipes-kernel/zephyr-kernel/zephyr-sample.inc

# Source files for our custom application
SRC_URI += "file://src/main.c \
            file://prj.conf \
            file://CMakeLists.txt \
           "

# Point to our application source directory
ZEPHYR_SRC_DIR = "${WORKDIR}"

# Zephyr board configuration for STM32F411CEU6 Black Pill
ZEPHYR_BOARD = "blackpill_f411ce"

# Compatible machine
COMPATIBLE_MACHINE = "stm32f411-dk"

# This is a Zephyr application that provides the kernel
PROVIDES = "virtual/kernel"

# Deploy with custom names
do_deploy:append() {
    # Create recovery-specific symlinks for easier identification
    if [ -f ${DEPLOYDIR}/${PN}.bin ]; then
        ln -sf ${PN}.bin ${DEPLOYDIR}/stm32f411-recovery.bin
    fi
    if [ -f ${DEPLOYDIR}/${PN}.elf ]; then
        ln -sf ${PN}.elf ${DEPLOYDIR}/stm32f411-recovery.elf
    fi
    if [ -f ${DEPLOYDIR}/${PN}.hex ]; then
        ln -sf ${PN}.hex ${DEPLOYDIR}/stm32f411-recovery.hex
    fi
}
