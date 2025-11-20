# Dockerfile for STM32F411 Yocto Build Environment
# Based on Ubuntu 22.04 LTS with all Yocto Project dependencies

FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Set locale to UTF-8 (required by Yocto)
RUN apt-get update && apt-get install -y locales && \
    locale-gen en_US.UTF-8 && \
    update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

# Install Yocto Project dependencies
# Reference: https://docs.yoctoproject.org/ref-manual/system-requirements.html
RUN apt-get update && apt-get install -y \
    # Essential build tools
    gawk wget git diffstat unzip texinfo gcc build-essential \
    chrpath socat cpio python3 python3-pip python3-pexpect \
    xz-utils debianutils iputils-ping python3-git python3-jinja2 \
    libegl1-mesa libsdl1.2-dev xterm python3-subunit mesa-common-dev \
    zstd liblz4-tool file lz4 \
    # Ninja (CMake will be installed separately with specific version)
    ninja-build \
    # Additional utilities
    vim nano curl ca-certificates sudo gosu \
    # ARM toolchain for Cortex-M4
    gcc-arm-none-eabi binutils-arm-none-eabi \
    # Python packages
    python3-setuptools python3-wheel \
    # Git LFS (if needed for large files)
    git-lfs \
    # Cleanup
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Install CMake 3.20.5 (compatible with Zephyr)
RUN cd /tmp && \
    wget -q https://github.com/Kitware/CMake/releases/download/v3.20.5/cmake-3.20.5-Linux-x86_64.sh && \
    chmod +x cmake-3.20.5-Linux-x86_64.sh && \
    ./cmake-3.20.5-Linux-x86_64.sh --skip-license --prefix=/usr/local && \
    rm cmake-3.20.5-Linux-x86_64.sh && \
    cmake --version

# Install KAS build tool
# Add timeout and retry settings to handle network issues
RUN pip3 install --default-timeout=100 --retries 5 --no-cache-dir kas

# Install additional Python packages that might be needed
RUN pip3 install --default-timeout=100 --retries 5 --no-cache-dir \
    GitPython \
    jinja2 \
    ply \
    west

# Install Zephyr SDK for ARM Cortex-M development
# This provides a complete toolchain specifically for Zephyr RTOS
# Note: Installing as root before creating the user
ARG ZEPHYR_SDK_VERSION=0.16.8
ARG ZEPHYR_SDK_INSTALL_DIR=/opt/zephyr-sdk-${ZEPHYR_SDK_VERSION}

RUN cd /tmp && \
    wget -q https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.xz && \
    mkdir -p /opt && \
    tar -xf zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.xz -C /opt && \
    rm zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.xz && \
    cd ${ZEPHYR_SDK_INSTALL_DIR} && \
    ./setup.sh -t arm-zephyr-eabi -h -c

# Set Zephyr SDK environment variables
ENV ZEPHYR_SDK_INSTALL_DIR=/opt/zephyr-sdk-0.16.8
ENV ZEPHYR_TOOLCHAIN_VARIANT=zephyr

# Create a non-root user for building (recommended for Yocto)
# This prevents permission issues with generated files
ARG USERNAME=yoctouser
ARG USER_UID=1000
ARG USER_GID=1000

RUN groupadd --gid $USER_GID $USERNAME && \
    useradd --uid $USER_UID --gid $USER_GID -m $USERNAME && \
    echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Set up working directory
WORKDIR /workspace

# Change ownership to the non-root user
RUN chown -R $USERNAME:$USERNAME /workspace

# Give yoctouser access to Zephyr SDK
RUN chown -R $USERNAME:$USERNAME /opt/zephyr-sdk-0.16.8

# Switch to non-root user
USER $USERNAME

# Set up Git configuration (customize these)
RUN git config --global user.name "Yocto Builder" && \
    git config --global user.email "builder@localhost" && \
    git config --global color.ui auto

# Environment variables for Yocto build optimization
ENV BB_ENV_PASSTHROUGH_ADDITIONS="SSTATE_DIR DL_DIR TMPDIR"

# Create entrypoint script to fix ownership issues
# This runs every time the container starts
USER root
RUN echo '#!/bin/bash\n\
set -e\n\
\n\
# Fix ownership of workspace to match the yoctouser\n\
echo "Fixing ownership of /workspace/meta-mono..."\n\
chown -R yoctouser:yoctouser /workspace/meta-mono 2>/dev/null || true\n\
\n\
# Execute command as yoctouser\n\
if [ "$1" = "bash" ] || [ "$1" = "/bin/bash" ] || [ -z "$1" ]; then\n\
    exec gosu yoctouser /bin/bash\n\
else\n\
    exec gosu yoctouser "$@"\n\
fi' > /entrypoint.sh && \
    chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]

# Default command
CMD ["/bin/bash"]
