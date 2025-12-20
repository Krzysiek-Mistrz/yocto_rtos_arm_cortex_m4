SUMMARY = "UART Bridge Daemon for i.MX6ULL <-> STM32F411 communication"
DESCRIPTION = "Daemon that manages UART communication between i.MX6ULL running \
Linux and STM32F411 running Zephyr RTOS. Provides Unix socket interface for \
local applications to communicate with the STM32."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://uart-bridge.c \
    file://uart-protocol.h \
    file://uart-bridge.service \
"

S = "${WORKDIR}"

inherit systemd

SYSTEMD_SERVICE:${PN} = "uart-bridge.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} -o uart-bridge uart-bridge.c
}

do_install() {
    # Install binary
    install -d ${D}${bindir}
    install -m 0755 uart-bridge ${D}${bindir}/

    # Install header (for other applications)
    install -d ${D}${includedir}
    install -m 0644 uart-protocol.h ${D}${includedir}/

    # Install systemd service
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 uart-bridge.service ${D}${systemd_system_unitdir}/
}

FILES:${PN} += " \
    ${bindir}/uart-bridge \
    ${systemd_system_unitdir}/uart-bridge.service \
"

FILES:${PN}-dev += " \
    ${includedir}/uart-protocol.h \
"

RDEPENDS:${PN} += "systemd"
