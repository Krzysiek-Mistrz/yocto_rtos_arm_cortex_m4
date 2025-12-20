FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Add custom device tree for EBYTE i.MX6ULL
SRC_URI:append:imx6ull-ebyte = " \
    file://imx6ull-ebyte-emmc.dts \
"

# Copy custom device tree before configuration/compilation
do_configure:prepend:imx6ull-ebyte() {
    if [ -f ${WORKDIR}/imx6ull-ebyte-emmc.dts ]; then
        cp ${WORKDIR}/imx6ull-ebyte-emmc.dts ${S}/arch/arm/boot/dts/nxp/imx/
    fi
}

KERNEL_DEVICETREE:imx6ull-ebyte = "nxp/imx/imx6ull-ebyte-emmc.dtb"


