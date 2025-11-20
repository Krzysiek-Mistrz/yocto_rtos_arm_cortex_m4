/*
 * SPI Test Utility for STM32F411 with HAL
 * Tests SPI communication using STM32 HAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;

void print_usage(const char *prog) {
    printf("Usage: %s <bus> <data...>\\n", prog);
    printf("  bus: 1, 2, or 3\\n");
    printf("  data: Bytes to transmit (hex, space-separated)\\n");
    printf("\\nExample: %s 1 0xAA 0xBB 0xCC 0xDD\\n", prog);
}

void SPI_Init(int bus) {
    SPI_HandleTypeDef *hspi;
    
    switch(bus) {
        case 1: hspi = &hspi1; hspi->Instance = SPI1; break;
        case 2: hspi = &hspi2; hspi->Instance = SPI2; break;
        case 3: hspi = &hspi3; hspi->Instance = SPI3; break;
        default: return;
    }
    
    hspi->Init.Mode = SPI_MODE_MASTER;
    hspi->Init.Direction = SPI_DIRECTION_2LINES;
    hspi->Init.DataSize = SPI_DATASIZE_8BIT;
    hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi->Init.NSS = SPI_NSS_SOFT;
    hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi->Init.TIMode = SPI_TIMODE_DISABLE;
    hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    
    if (HAL_SPI_Init(hspi) != HAL_OK) {
        printf("Error: SPI%d initialization failed\\n", bus);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    int bus = atoi(argv[1]);
    int data_count = argc - 2;

    if (bus < 1 || bus > 3) {
        printf("Error: Invalid SPI bus %d\\n", bus);
        return 1;
    }

    printf("SPI Test Utility for STM32F411 (HAL)\\n");
    printf("Bus: SPI%d\\n", bus);
    printf("Transmitting %d bytes: ", data_count);

    uint8_t tx_data[256];
    uint8_t rx_data[256];
    
    for (int i = 0; i < data_count && i < 256; i++) {
        tx_data[i] = (uint8_t)strtol(argv[2 + i], NULL, 16);
        printf("0x%02X ", tx_data[i]);
    }
    printf("\\n\\n");

    /* Initialize HAL */
    HAL_Init();
    SPI_Init(bus);

    /* Get the correct SPI handle */
    SPI_HandleTypeDef *hspi;
    switch(bus) {
        case 1: hspi = &hspi1; break;
        case 2: hspi = &hspi2; break;
        case 3: hspi = &hspi3; break;
        default: return 1;
    }

    /* Perform SPI transfer */
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, tx_data, rx_data, 
                                                        data_count, 1000);

    if (status == HAL_OK) {
        printf("Transfer successful!\\n");
        printf("Received data: ");
        for (int i = 0; i < data_count; i++) {
            printf("0x%02X ", rx_data[i]);
        }
        printf("\\n");
    } else {
        printf("Error: SPI transfer failed (status: %d)\\n", status);
        return 1;
    }

    /* Deinitialize */
    HAL_SPI_DeInit(hspi);
    
    return 0;
}
