/*
 * System Information Utility for STM32F411 with HAL
 * Displays system and hardware information
 */

#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"

void print_clock_info(void) {
    printf("Clock Configuration:\n");
    printf("  SYSCLK: %lu Hz (%lu MHz)\n", 
           HAL_RCC_GetSysClockFreq(), HAL_RCC_GetSysClockFreq() / 1000000);
    printf("  HCLK: %lu Hz (%lu MHz)\n", 
           HAL_RCC_GetHCLKFreq(), HAL_RCC_GetHCLKFreq() / 1000000);
    printf("  PCLK1: %lu Hz (%lu MHz)\n", 
           HAL_RCC_GetPCLK1Freq(), HAL_RCC_GetPCLK1Freq() / 1000000);
    printf("  PCLK2: %lu Hz (%lu MHz)\n", 
           HAL_RCC_GetPCLK2Freq(), HAL_RCC_GetPCLK2Freq() / 1000000);
}

void print_device_info(void) {
    uint32_t device_id = HAL_GetDEVID();
    uint32_t revision_id = HAL_GetREVID();
    uint32_t uid[3];
    
    /* Read unique device ID */
    uid[0] = *(uint32_t*)(UID_BASE);
    uid[1] = *(uint32_t*)(UID_BASE + 4);
    uid[2] = *(uint32_t*)(UID_BASE + 8);

    printf("Device Information:\n");
    printf("  Device ID: 0x%03lX\n", device_id);
    printf("  Revision ID: 0x%04lX\n", revision_id);
    printf("  Unique ID: %08lX-%08lX-%08lX\n", uid[0], uid[1], uid[2]);
    printf("  Flash Size: %u KB\n", *(uint16_t*)FLASHSIZE_BASE);
}

void print_peripheral_status(void) {
    printf("\nPeripheral Status:\n");
    
    /* Check which peripherals are enabled */
    printf("  GPIO Ports:\n");
    printf("    GPIOA: %s\n", __HAL_RCC_GPIOA_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    GPIOB: %s\n", __HAL_RCC_GPIOB_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    GPIOC: %s\n", __HAL_RCC_GPIOC_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    GPIOD: %s\n", __HAL_RCC_GPIOD_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    GPIOE: %s\n", __HAL_RCC_GPIOE_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    GPIOH: %s\n", __HAL_RCC_GPIOH_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    
    printf("  Communication Interfaces:\n");
    printf("    I2C1: %s\n", __HAL_RCC_I2C1_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    I2C2: %s\n", __HAL_RCC_I2C2_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    I2C3: %s\n", __HAL_RCC_I2C3_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    SPI1: %s\n", __HAL_RCC_SPI1_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    SPI2: %s\n", __HAL_RCC_SPI2_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    SPI3: %s\n", __HAL_RCC_SPI3_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    USART1: %s\n", __HAL_RCC_USART1_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    USART2: %s\n", __HAL_RCC_USART2_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
    printf("    USART6: %s\n", __HAL_RCC_USART6_IS_CLK_ENABLED() ? "Enabled" : "Disabled");
}

void print_memory_info(void) {
    printf("\nMemory Information:\n");
    printf("  Flash:\n");
    printf("    Base: 0x08000000\n");
    printf("    Size: 512 KB\n");
    printf("  SRAM:\n");
    printf("    Base: 0x20000000\n");
    printf("    Size: 128 KB\n");
}

void print_reset_cause(void) {
    printf("\nReset Cause:\n");
    
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)) {
        printf("  Low-power reset\n");
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)) {
        printf("  Window watchdog reset\n");
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
        printf("  Independent watchdog reset\n");
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) {
        printf("  Software reset\n");
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST)) {
        printf("  Power-on reset\n");
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) {
        printf("  Pin reset (NRST)\n");
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST)) {
        printf("  Brown-out reset\n");
    }
    
    /* Clear reset flags */
    __HAL_RCC_CLEAR_RESET_FLAGS();
}

int main(void) {
    HAL_Init();

    printf("========================================\n");
    printf("STM32F411 System Information (HAL)\n");
    printf("========================================\n\n");

    print_device_info();
    printf("\n");
    print_clock_info();
    print_peripheral_status();
    print_memory_info();
    print_reset_cause();

    printf("\n========================================\n");

    return 0;
}
