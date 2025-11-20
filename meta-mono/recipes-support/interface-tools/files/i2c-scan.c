/*
 * I2C Scanner for STM32F411 with HAL
 * Scans I2C bus for connected devices using STM32 HAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "stm32f4xx_hal.h"

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;

void print_usage(const char *prog) {
    printf("Usage: %s <bus>\n", prog);
    printf("  bus: 1, 2, or 3\n");
    printf("\nScans I2C bus for connected devices\n");
}

void I2C_Init(int bus) {
    I2C_HandleTypeDef *hi2c;
    
    switch(bus) {
        case 1: hi2c = &hi2c1; hi2c->Instance = I2C1; break;
        case 2: hi2c = &hi2c2; hi2c->Instance = I2C2; break;
        case 3: hi2c = &hi2c3; hi2c->Instance = I2C3; break;
        default: return;
    }
    
    hi2c->Init.ClockSpeed = 100000;  /* 100kHz */
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

int i2c_probe(I2C_HandleTypeDef *hi2c, uint8_t addr) {
    /* Use HAL function to check if device is ready */
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(hi2c, addr << 1, 1, 10);
    return (status == HAL_OK);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    int bus = atoi(argv[1]);
    
    if (bus < 1 || bus > 3) {
        printf("Error: Invalid I2C bus %d\n", bus);
        return 1;
    }
    
    /* Initialize HAL */
    HAL_Init();
    I2C_Init(bus);
    
    I2C_HandleTypeDef *hi2c = get_i2c_handle(bus);
    if (!hi2c) {
        printf("Error: Failed to get I2C handle\n");
        return 1;
    }

    printf("I2C Scanner for STM32F411 (HAL)\n");
    printf("Scanning I2C%d bus...\n\n", bus);
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

    int found = 0;
    for (int addr = 0; addr < 128; addr++) {
        if (addr % 16 == 0) {
            printf("%02x: ", addr);
        }

        /* Skip reserved addresses */
        if (addr < 0x03 || addr > 0x77) {
            printf("   ");
        } else {
            if (i2c_probe(hi2c, addr)) {
                printf("%02x ", addr);
                found++;
            } else {
                printf("-- ");
            }
        }

        if ((addr + 1) % 16 == 0) {
            printf("\n");
        }
    }

    printf("\nFound %d device(s) on I2C%d\n", found, bus);
    
    /* Deinitialize */
    HAL_I2C_DeInit(hi2c);
    
    return 0;
}
