SUMMARY = "GPIO manipulation tools for STM32F411"
DESCRIPTION = "Command-line utilities for GPIO control, monitoring, and testing on STM32F411"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://gpio-test.c \
           file://gpio-monitor.c \
           file://gpio-set.c \
           file://Makefile \
          "

S = "${WORKDIR}"

DEPENDS = "stm32f4-hal"

do_compile() {
    oe_runmake
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 gpio-test ${D}${bindir}/
    install -m 0755 gpio-monitor ${D}${bindir}/
    install -m 0755 gpio-set ${D}${bindir}/
}

FILES:${PN} = "${bindir}/*"

COMPATIBLE_MACHINE = "stm32f411-dk"
