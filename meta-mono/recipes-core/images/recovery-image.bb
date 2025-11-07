DESCRIPTION = "STM32F411 Recovery System with GPIO and Interface Tools"
LICENSE = "MIT"

# Inherit core image class for embedded systems
inherit core-image

# Essential packages for STM32F411 recovery and development
IMAGE_INSTALL = "zephyr-kernel \
                stm32f4-hal \
                gpio-tools \
                interface-tools \
                recovery-tools \
                "

# Minimal features for embedded system
IMAGE_FEATURES = "tools-debug"

# Remove unnecessary features
IMAGE_FEATURES:remove = "package-management splash"

# Image formats for STM32F411
IMAGE_FSTYPES = "bin elf hex"

# Image name
IMAGE_NAME = "${IMAGE_BASENAME}-${MACHINE}${IMAGE_NAME_SUFFIX}"

# Size optimization
IMAGE_OVERHEAD_FACTOR = "1.0"
IMAGE_ROOTFS_EXTRA_SPACE = "0"