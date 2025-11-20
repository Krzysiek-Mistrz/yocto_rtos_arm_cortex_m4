/*
 * I2C Write Utility for STM32F411 with HAL
 * Writes data to I2C device using STM32 HAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;

void print_usage(const char *prog) {
    printf("Usage: %s <bus> <device_addr> <register> <data...>\n", prog);
    printf("  bus: 1, 2, or 3\n");
    printf("  device_addr: I2C device address (hex)\n");
    printf("  register: Register address to write to (hex)\n");
    printf("  data: Bytes to write (hex, space-separated)\n");
    printf("\nExample: %s 1 0x50 0x00 0xAA 0xBB 0xCC\n", prog);
}

void I2C_Init(int bus) {
    I2C_HandleTypeDef *hi2c;
    
    switch(bus) {
        case 1: hi2c = &hi2c1; hi2c->Instance = I2C1; break;
        case 2: hi2c = &hi2c2; hi2c->Instance = I2C2; break;
        case 3: hi2c = &hi2c3; hi2c->Instance = I2C3; break;
        default: return;
    }
    
    hi2c->Init.ClockSpeed = 100000;
    hi2c->Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c->Init.OwnAddress1 = 0;
    hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(hi2c) != HAL_OK) {
        printf("Error: I2C%d initialization failed\n", bus);
    }
}

I2C_HandleTypeDef* get_i2c_handle(int bus) {
    switch(bus) {
        case 1: return &hi2c1;
        case 2: return &hi2c2;
        case 3: return &hi2c3;
        default: return NULL;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        print_usage(argv[0]);
        return 1;
    }

    int bus = atoi(argv[1]);
    uint8_t dev_addr = (uint8_t)strtol(argv[2], NULL, 16);
    uint8_t reg_addr = (uint8_t)strtol(argv[3], NULL, 16);
    int data_count = argc - 4;

    if (bus < 1 || bus > 3) {
        printf("Error: Invalid I2C bus %d\n", bus);
        return 1;
    }

    if (data_count > 256) {
        printf("Error: Too much data (max 256 bytes)\n");
        return 1;
    }

    printf("I2C Write Utility for STM32F411 (HAL)\n");
    printf("Bus: I2C%d, Device: 0x%02X, Register: 0x%02X\n",
           bus, dev_addr, reg_addr);
    printf("Writing %d bytes: ", data_count);

    uint8_t tx_data[256];
    for (int i = 0; i < data_count; i++) {
        tx_data[i] = (uint8_t)strtol(argv[4 + i], NULL, 16);
        printf("0x%02X ", tx_data[i]);
    }
    printf("\n\n");

    /* Initialize HAL */
    HAL_Init();
    I2C_Init(bus);
    
    I2C_HandleTypeDef *hi2c = get_i2c_handle(bus);
    if (!hi2c) {
        printf("Error: Failed to get I2C handle\n");
        return 1;
    }

    /* Write data to register */
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(hi2c, dev_addr << 1, reg_addr,
                                                  I2C_MEMADD_SIZE_8BIT,
                                                  tx_data, data_count, 1000);

    if (status == HAL_OK) {
        printf("Write successful!\n");
    } else {
        printf("Error: I2C write failed (status: %d)\n", status);
        HAL_I2C_DeInit(hi2c);
        return 1;
    }

    /* Deinitialize */
    HAL_I2C_DeInit(hi2c);
    
    return 0;
}
