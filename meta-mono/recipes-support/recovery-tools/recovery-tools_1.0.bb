SUMMARY = "Recovery and diagnostic tools for STM32F411"
DESCRIPTION = "Flash programming, memory testing, and system recovery utilities for STM32F411"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://flash-program.c \
           file://memory-test.c \
           file://system-info.c \
           file://Makefile \
          "

S = "${WORKDIR}"

DEPENDS = "stm32f4-hal"

do_compile() {
    oe_runmake
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 flash-program ${D}${bindir}/
    install -m 0755 memory-test ${D}${bindir}/
    install -m 0755 system-info ${D}${bindir}/
}

FILES:${PN} = "${bindir}/*"

COMPATIBLE_MACHINE = "stm32f411-dk"
