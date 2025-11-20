SUMMARY = "STM32F4 Hardware Abstraction Layer"
DESCRIPTION = "STMicroelectronics STM32F4xx HAL and LL drivers for STM32F411"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE.md;md5=8b4e8e5e0c7b3c3e3e3e3e3e3e3e3e3e"

# STM32CubeF4 repository - using release tarball to avoid git fetch issues
SRC_URI = "https://github.com/STMicroelectronics/STM32CubeF4/archive/refs/tags/v${PV}.tar.gz"
SRC_URI[sha256sum] = ""

S = "${WORKDIR}/STM32CubeF4-${PV}"

inherit cmake

# Package configuration
PROVIDES = "stm32f4-hal"
RPROVIDES:${PN} = "stm32f4-hal"

# Include paths for HAL
EXTRA_OECMAKE = " \
    -DSTM32_CHIP=STM32F411xE \
    -DSTM32_FAMILY=F4 \
    -DCMAKE_BUILD_TYPE=Release \
"

# Install HAL headers and libraries
do_install() {
    install -d ${D}${includedir}/stm32f4xx
    install -d ${D}${libdir}

    # Install HAL headers
    install -m 0644 ${S}/Drivers/STM32F4xx_HAL_Driver/Inc/*.h ${D}${includedir}/stm32f4xx/
    install -m 0644 ${S}/Drivers/CMSIS/Device/ST/STM32F4xx/Include/*.h ${D}${includedir}/stm32f4xx/
    install -m 0644 ${S}/Drivers/CMSIS/Include/*.h ${D}${includedir}/stm32f4xx/

    # Install compiled library (if built)
    if [ -f ${B}/libstm32f4_hal.a ]; then
        install -m 0644 ${B}/libstm32f4_hal.a ${D}${libdir}/libhal_stm32f4.a
    fi
}

# Package files
FILES:${PN} = "${libdir}/*.a"
FILES:${PN}-dev = "${includedir}/stm32f4xx/*.h"

COMPATIBLE_MACHINE = "stm32f411-dk"

# Dependencies
DEPENDS = "virtual/arm-none-eabi-gcc"
