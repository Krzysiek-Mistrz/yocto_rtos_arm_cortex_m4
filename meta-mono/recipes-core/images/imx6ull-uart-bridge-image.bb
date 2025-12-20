SUMMARY = "i.MX6ULL Linux image with UART bridge for STM32F411 communication"
DESCRIPTION = "Custom Linux image for EBYTE i.MX6ULL SoM with UART bridge daemon \
for communicating with STM32F411 running Zephyr RTOS"

LICENSE = "MIT"

# Inherit from core-image-base for minimal Linux system
inherit core-image

# Core packages
IMAGE_INSTALL:append = " \
    packagegroup-core-boot \
    packagegroup-core-full-cmdline \
    kernel-modules \
    linux-firmware \
"

# System utilities
IMAGE_INSTALL:append = " \
    systemd \
    systemd-analyze \
    util-linux \
    coreutils \
    procps \
    htop \
    nano \
    vim \
"

# Network tools
IMAGE_INSTALL:append = " \
    iproute2 \
    iputils \
    openssh \
    openssh-sftp-server \
"

# Development tools (can be removed for production)
IMAGE_INSTALL:append = " \
    gdb \
    strace \
    tcpdump \
    i2c-tools \
    spitools \
"

# UART bridge daemon for STM32F411 communication
IMAGE_INSTALL:append = " \
    uart-bridge \
"

# Additional features
IMAGE_FEATURES += " \
    ssh-server-openssh \
    tools-debug \
    debug-tweaks \
"

# Set root password to 'root' for development (remove for production)
EXTRA_USERS_PARAMS = "usermod -P root root;"

# Image size (in KB) - adjust as needed
IMAGE_ROOTFS_SIZE ?= "8192"
IMAGE_ROOTFS_EXTRA_SPACE = "512000"

# Enable serial console on UART1 (ttymxc0)
# UART2 (ttymxc1) is reserved for STM32F411 communication
