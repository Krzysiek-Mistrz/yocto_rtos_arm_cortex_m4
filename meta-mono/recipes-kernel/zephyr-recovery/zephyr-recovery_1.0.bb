SUMMARY = "STM32F411CEU6 Recovery System - Zephyr RTOS Application"
DESCRIPTION = "Zephyr-based recovery and diagnostic system for STM32F411CEU6 (Black Pill) with GPIO, I2C, SPI, and UART tools"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

# Inherit deploy class for binary deployment
inherit deploy

# Inherit python3native to make native Python modules (pyelftools, pyyaml) visible
# This sets up PYTHONPATH so Zephyr build scripts can find the modules
inherit python3native

# Source files for our custom application
SRC_URI = "file://src/main.c \
           file://prj.conf \
           file://CMakeLists.txt \
           file://app.overlay \
           file://kconfig.fragment \
          "

# Point to our application source directory
# Files from file:// URIs are extracted directly to WORKDIR
S = "${WORKDIR}"

# Depend on Zephyr kernel source and Python tools needed by Zephyr 3.6 build scripts
DEPENDS += "zephyr-kernel-src cmake-native ninja-native dtc-native \
            python3-pyelftools-native python3-pyyaml-native python3-pykwalify-native"

# Set up Zephyr environment
ZEPHYR_BASE = "${STAGING_DIR_TARGET}/usr/src/zephyr/zephyr"
export ZEPHYR_BASE

# Zephyr modules path - must include all required modules
# Zephyr needs explicit module paths in cross-compilation scenarios
ZEPHYR_MODULES = "${STAGING_DIR_TARGET}/usr/src/zephyr/modules/hal/cmsis;${STAGING_DIR_TARGET}/usr/src/zephyr/modules/hal/stm32;${STAGING_DIR_TARGET}/usr/src/zephyr/modules/hal/st"
export ZEPHYR_MODULES

# Zephyr board configuration for STM32F411CEU6 Black Pill
# Use the official Zephyr board definition
ZEPHYR_BOARD = "blackpill_f411ce"
export ZEPHYR_BOARD

# Build directory
B = "${WORKDIR}/build"

# Compatible machine
COMPATIBLE_MACHINE = "stm32f411-dk"

# Zephyr SDK 0.16.8 uses GCC 12.2.0 which doesn't support -fcanon-prefix-map
# This flag was added in GCC 13+, so we need to disable it
# Use only the older debug prefix map flags that GCC 12 supports
DEBUG_PREFIX_MAP = "-fmacro-prefix-map=${WORKDIR}=/usr/src/debug/${PN}/${EXTENDPE}${PV}-${PR} \
                    -fdebug-prefix-map=${WORKDIR}=/usr/src/debug/${PN}/${EXTENDPE}${PV}-${PR}"

# This is a Zephyr application that provides the kernel
PROVIDES = "virtual/kernel"

# Configure using Zephyr's CMake
do_configure() {
    # Set up Zephyr SDK toolchain environment
    # Using Zephyr SDK provides a complete, tested toolchain for Zephyr RTOS
    export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
    export ZEPHYR_SDK_INSTALL_DIR=/opt/zephyr-sdk-0.16.8
    
    echo "Using Zephyr SDK toolchain"
    echo "ZEPHYR_TOOLCHAIN_VARIANT: ${ZEPHYR_TOOLCHAIN_VARIANT}"
    echo "ZEPHYR_SDK_INSTALL_DIR: ${ZEPHYR_SDK_INSTALL_DIR}"
    
    # Verify Zephyr SDK is available
    if [ ! -d "${ZEPHYR_SDK_INSTALL_DIR}" ]; then
        bbfatal "Zephyr SDK not found at ${ZEPHYR_SDK_INSTALL_DIR}. Please rebuild the Docker image."
    fi
    
    # Create build directory
    mkdir -p ${B}
    
    # Clear Zephyr cache to avoid CMake version compatibility issues
    # See: https://github.com/zephyrproject-rtos/zephyr/pull/30272
    rm -rf ${WORKDIR}/.cache/zephyr 2>/dev/null || true
    rm -rf /tmp/tmp*/.cache/zephyr 2>/dev/null || true
    
    # Ensure ZEPHYR_MODULES is set (should be inherited from recipe-level export)
    # But we can also set it here explicitly to be sure
    export ZEPHYR_MODULES="${STAGING_DIR_TARGET}/usr/src/zephyr/modules/hal/cmsis;${STAGING_DIR_TARGET}/usr/src/zephyr/modules/hal/stm32;${STAGING_DIR_TARGET}/usr/src/zephyr/modules/hal/st"
    
    # Debug: Print Zephyr configuration
    echo "ZEPHYR_BASE: ${ZEPHYR_BASE}"
    echo "ZEPHYR_MODULES: ${ZEPHYR_MODULES}"
    echo "ZEPHYR_BOARD: ${ZEPHYR_BOARD}"
    
    # Verify hal_stm32 exists
    if [ -d "${STAGING_DIR_TARGET}/usr/src/zephyr/modules/hal/stm32" ]; then
        echo "hal_stm32 found at: ${STAGING_DIR_TARGET}/usr/src/zephyr/modules/hal/stm32"
        ls -la "${STAGING_DIR_TARGET}/usr/src/zephyr/modules/hal/stm32" || true
    else
        bbwarn "hal_stm32 module not found! Build may fail."
    fi
    
    # Run CMake from the build directory
    cd ${B}
    
    # Use system cmake (3.17.x) instead of Yocto's cmake-native (3.31.x)
    # Zephyr requires CMake 3.17-3.18 for proper toolchain detection
    /usr/local/bin/cmake -GNinja \
        -DBOARD=${ZEPHYR_BOARD} \
        -DZEPHYR_BASE=${ZEPHYR_BASE} \
        -DZEPHYR_MODULES="${ZEPHYR_MODULES}" \
        -DZEPHYR_TOOLCHAIN_VARIANT=zephyr \
        -DZEPHYR_SDK_INSTALL_DIR=${ZEPHYR_SDK_INSTALL_DIR} \
        -DDTC_OVERLAY_FILE=${S}/app.overlay \
        -B ${B} \
        -S ${S}
}

# Compile using ninja
do_compile() {
    cd ${B}
    ninja -v
}

# Install is not needed for embedded firmware
do_install() {
    :
}

# Deploy the built binaries
do_deploy() {
    install -d ${DEPLOYDIR}
    
    # Deploy the main binaries
    if [ -f ${B}/zephyr/zephyr.bin ]; then
        install -m 0644 ${B}/zephyr/zephyr.bin ${DEPLOYDIR}/${PN}.bin
    fi
    if [ -f ${B}/zephyr/zephyr.elf ]; then
        install -m 0644 ${B}/zephyr/zephyr.elf ${DEPLOYDIR}/${PN}.elf
    fi
    if [ -f ${B}/zephyr/zephyr.hex ]; then
        install -m 0644 ${B}/zephyr/zephyr.hex ${DEPLOYDIR}/${PN}.hex
    fi
    
    # Create recovery-specific symlinks
    if [ -f ${DEPLOYDIR}/${PN}.bin ]; then
        ln -sf ${PN}.bin ${DEPLOYDIR}/stm32f411-recovery.bin
    fi
    if [ -f ${DEPLOYDIR}/${PN}.elf ]; then
        ln -sf ${PN}.elf ${DEPLOYDIR}/stm32f411-recovery.elf
    fi
    if [ -f ${DEPLOYDIR}/${PN}.hex ]; then
        ln -sf ${PN}.hex ${DEPLOYDIR}/stm32f411-recovery.hex
    fi
}

addtask deploy after do_compile before do_build
