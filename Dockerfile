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
    # Additional utilities
    vim nano curl ca-certificates \
    # ARM toolchain for Cortex-M4
    gcc-arm-none-eabi binutils-arm-none-eabi \
    # Python packages
    python3-setuptools python3-wheel \
    # Git LFS (if needed for large files)
    git-lfs \
    # Cleanup
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Install KAS build tool
RUN pip3 install --no-cache-dir kas

# Install additional Python packages that might be needed
RUN pip3 install --no-cache-dir \
    GitPython \
    jinja2 \
    ply

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

# Switch to non-root user
USER $USERNAME

# Set up Git configuration (customize these)
RUN git config --global user.name "Yocto Builder" && \
    git config --global user.email "builder@localhost" && \
    git config --global color.ui auto

# Environment variables for Yocto build optimization
ENV BB_ENV_PASSTHROUGH_ADDITIONS="SSTATE_DIR DL_DIR TMPDIR"

# Default command
CMD ["/bin/bash"]
