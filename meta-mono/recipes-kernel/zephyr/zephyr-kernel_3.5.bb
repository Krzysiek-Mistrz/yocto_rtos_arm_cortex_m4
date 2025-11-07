SUMMARY = "Zephyr RTOS Kernel for STM32F411"
DESCRIPTION = "Zephyr Real-Time Operating System kernel configured for STM32F411 with recovery and GPIO tools"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

inherit zephyr-kernel

# Zephyr version
PV = "3.5.0"

# Source repository
SRC_URI = "git://github.com/zephyrproject-rtos/zephyr.git;protocol=https;branch=main \
           file://stm32f411_defconfig \
           file://stm32f411-dk.dts \
          "

SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

# Board configuration
ZEPHYR_BOARD = "stm32f411_dk"
ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"

# Extra configuration
ZEPHYR_EXTRA_MODULES = ""

# Install defconfig and device tree
do_configure:prepend() {
    install -d ${S}/boards/arm/stm32f411_dk
    install -m 0644 ${WORKDIR}/stm32f411_defconfig ${S}/boards/arm/stm32f411_dk/stm32f411_dk_defconfig
    install -m 0644 ${WORKDIR}/stm32f411-dk.dts ${S}/boards/arm/stm32f411_dk/stm32f411_dk.dts
}

# Build outputs
do_deploy() {
    install -d ${DEPLOYDIR}
    install -m 0644 ${B}/zephyr/zephyr.bin ${DEPLOYDIR}/zephyr-${MACHINE}.bin
    install -m 0644 ${B}/zephyr/zephyr.elf ${DEPLOYDIR}/zephyr-${MACHINE}.elf
    install -m 0644 ${B}/zephyr/zephyr.hex ${DEPLOYDIR}/zephyr-${MACHINE}.hex
}

addtask deploy after do_compile before do_build

FILES:${PN} = "/boot/*"

COMPATIBLE_MACHINE = "stm32f411-dk"
