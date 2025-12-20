FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Additional U-Boot configuration for EBYTE board
#
# NOTE: Do not set UBOOT_MACHINE here. The machine configuration uses
# UBOOT_CONFIG and setting both variables causes u-boot to be skipped.
SRC_URI:append:imx6ull-ebyte = " \
"

# Boot environment settings
UBOOT_ENV_SUFFIX:imx6ull-ebyte = "bin"
