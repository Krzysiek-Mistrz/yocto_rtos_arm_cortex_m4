SUMMARY = "Interface communication tools for STM32F411"
DESCRIPTION = "Command-line utilities for I2C, SPI, and UART communication testing on STM32F411"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://i2c-scan.c \
           file://i2c-read.c \
           file://i2c-write.c \
           file://spi-test.c \
           file://uart-test.c \
           file://Makefile \
          "

S = "${WORKDIR}"

DEPENDS = "stm32f4-hal"

do_compile() {
    oe_runmake
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 i2c-scan ${D}${bindir}/
    install -m 0755 i2c-read ${D}${bindir}/
    install -m 0755 i2c-write ${D}${bindir}/
    install -m 0755 spi-test ${D}${bindir}/
    install -m 0755 uart-test ${D}${bindir}/
}

FILES:${PN} = "${bindir}/*"

COMPATIBLE_MACHINE = "stm32f411-dk"
