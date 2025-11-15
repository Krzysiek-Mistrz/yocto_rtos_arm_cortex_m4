#!/bin/bash
# Docker Build Script for STM32F411 Yocto Project
# This script simplifies building the Yocto image inside Docker

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored message
print_msg() {
    echo -e "${2}${1}${NC}"
}

print_msg "========================================" "$BLUE"
print_msg "STM32F411 Yocto Docker Build Script" "$BLUE"
print_msg "========================================" "$BLUE"
echo ""

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    print_msg "ERROR: Docker is not installed!" "$RED"
    print_msg "Please install Docker first: https://docs.docker.com/get-docker/" "$YELLOW"
    exit 1
fi

# Check if docker compose is installed
if ! command -v docker compose &> /dev/null; then
    print_msg "WARNING: docker compose is not installed!" "$YELLOW"
    print_msg "You can still use 'docker build' and 'docker run' manually." "$YELLOW"
    USE_COMPOSE=false
else
    USE_COMPOSE=true
fi

# Function to build Docker image
build_image() {
    print_msg "Building Docker image..." "$GREEN"
    
    # Get current user UID and GID to avoid permission issues
    USER_UID=$(id -u)
    USER_GID=$(id -g)
    
    print_msg "Using UID=$USER_UID, GID=$USER_GID" "$BLUE"
    
    docker build \
        --build-arg USERNAME=yoctouser \
        --build-arg USER_UID=$USER_UID \
        --build-arg USER_GID=$USER_GID \
        -t stm32f411-yocto-builder:latest \
        -f Dockerfile .
    
    print_msg "Docker image built successfully!" "$GREEN"
}

# Function to start container with docker compose
start_compose() {
    print_msg "Starting container with docker compose..." "$GREEN"
    
    # Update docker compose.yml with current user UID/GID
    USER_UID=$(id -u)
    USER_GID=$(id -g)
    
    # Create cache directories if they don't exist
    mkdir -p yocto-cache/downloads yocto-cache/sstate-cache
    
    docker compose up -d
    print_msg "Container started! Use 'docker compose exec yocto-builder bash' to enter." "$GREEN"
}

# Function to run container manually
run_manual() {
    print_msg "Starting container manually..." "$GREEN"
    
    # Create cache directories if they don't exist
    mkdir -p yocto-cache/downloads yocto-cache/sstate-cache
    
    docker run -it --rm \
        --name stm32f411-yocto-builder \
        -v "$(pwd):/workspace:rw" \
        -v "$(pwd)/yocto-cache/downloads:/workspace/meta-mono/build/downloads:rw" \
        -v "$(pwd)/yocto-cache/sstate-cache:/workspace/meta-mono/build/sstate-cache:rw" \
        -e BB_NUMBER_THREADS=8 \
        -e PARALLEL_MAKE=-j8 \
        -e DL_DIR=/workspace/meta-mono/build/downloads \
        -e SSTATE_DIR=/workspace/meta-mono/build/sstate-cache \
        -e LANG=en_US.UTF-8 \
        -e LC_ALL=en_US.UTF-8 \
        --cpus=8 \
        --memory=16g \
        -w /workspace \
        stm32f411-yocto-builder:latest \
        /bin/bash
}

# Function to build Yocto image inside container
build_yocto() {
    print_msg "Building Yocto image for STM32F411..." "$GREEN"
    
    if [ "$USE_COMPOSE" = true ]; then
        docker compose exec yocto-builder bash -c "cd /workspace/meta-mono && kas build kas/firmware.yaml"
    else
        print_msg "Please run this inside the container:" "$YELLOW"
        print_msg "  cd /workspace/meta-mono && kas build kas/firmware.yaml" "$BLUE"
    fi
}

# Main menu
echo "What would you like to do?"
echo ""
echo "1) Build Docker image"
echo "2) Start container (interactive shell)"
echo "3) Build Yocto image (inside container)"
echo "4) Build Docker image AND start container"
echo "5) Full build (Docker image + Yocto image)"
echo "6) Exit"
echo ""
read -p "Enter your choice [1-6]: " choice

case $choice in
    1)
        build_image
        ;;
    2)
        if [ "$USE_COMPOSE" = true ]; then
            start_compose
        else
            run_manual
        fi
        ;;
    3)
        build_yocto
        ;;
    4)
        build_image
        if [ "$USE_COMPOSE" = true ]; then
            start_compose
        else
            run_manual
        fi
        ;;
    5)
        build_image
        print_msg "Starting container and building Yocto image..." "$GREEN"
        
        if [ "$USE_COMPOSE" = true ]; then
            docker compose up -d
            docker compose exec yocto-builder bash -c "cd /workspace/meta-mono && kas build kas/firmware.yaml"
        else
            print_msg "Please start the container manually and run:" "$YELLOW"
            print_msg "  cd /workspace/meta-mono && kas build kas/firmware.yaml" "$BLUE"
        fi
        ;;
    6)
        print_msg "Exiting..." "$BLUE"
        exit 0
        ;;
    *)
        print_msg "Invalid choice!" "$RED"
        exit 1
        ;;
esac

print_msg "" "$NC"
print_msg "========================================" "$BLUE"
print_msg "Done!" "$GREEN"
print_msg "========================================" "$BLUE"
