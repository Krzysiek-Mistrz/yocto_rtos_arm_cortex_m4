/*
 * UART Test Utility for STM32F411 with HAL
 * Tests UART communication using STM32 HAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

void print_usage(const char *prog) {
    printf("Usage: %s <uart> <mode> [data]\\n", prog);
    printf("  uart: 1, 2, or 6\\n");
    printf("  mode: send, receive, or loopback\\n");
    printf("  data: Text to send (for send mode)\\n");
    printf("\\nExamples:\\n");
    printf("  %s 2 send \"Hello World\"\\n", prog);
    printf("  %s 1 receive\\n", prog);
    printf("  %s 6 loopback\\n", prog);
}

void UART_Init(int uart, uint32_t baudrate) {
    UART_HandleTypeDef *huart;
    
    switch(uart) {
        case 1: huart = &huart1; huart->Instance = USART1; break;
        case 2: huart = &huart2; huart->Instance = USART2; break;
        case 6: huart = &huart6; huart->Instance = USART6; break;
        default: return;
    }
    
    huart->Init.BaudRate = baudrate;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(huart) != HAL_OK) {
        printf("Error: UART%d initialization failed\\n", uart);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    int uart = atoi(argv[1]);
    char *mode = argv[2];

    if (uart != 1 && uart != 2 && uart != 6) {
        printf("Error: Invalid UART %d\\n", uart);
        return 1;
    }

    printf("UART Test Utility for STM32F411 (HAL)\\n");
    printf("UART: USART%d, Mode: %s\\n\\n", uart, mode);

    /* Initialize HAL */
    HAL_Init();
    UART_Init(uart, 115200);

    /* Get the correct UART handle */
    UART_HandleTypeDef *huart;
    switch(uart) {
        case 1: huart = &huart1; break;
        case 2: huart = &huart2; break;
        case 6: huart = &huart6; break;
        default: return 1;
    }

    if (strcmp(mode, "send") == 0) {
        if (argc < 4) {
            printf("Error: No data provided for send mode\\n");
            return 1;
        }
        
        char *data = argv[3];
        printf("Sending: \\\"%s\\\"\\n", data);
        
        HAL_StatusTypeDef status = HAL_UART_Transmit(huart, (uint8_t*)data, 
                                                      strlen(data), 1000);
        
        if (status == HAL_OK) {
            printf("Data sent successfully!\\n");
        } else {
            printf("Error: UART transmit failed (status: %d)\\n", status);
            return 1;
        }
        
    } else if (strcmp(mode, "receive") == 0) {
        uint8_t rx_buffer[256];
        printf("Waiting to receive data (timeout: 5s)...\\n");
        
        HAL_StatusTypeDef status = HAL_UART_Receive(huart, rx_buffer, 
                                                     sizeof(rx_buffer), 5000);
        
        if (status == HAL_OK) {
            printf("Received: \\\"%s\\\"\\n", rx_buffer);
        } else if (status == HAL_TIMEOUT) {
            printf("Timeout: No data received\\n");
        } else {
            printf("Error: UART receive failed (status: %d)\\n", status);
            return 1;
        }
        
    } else if (strcmp(mode, "loopback") == 0) {
        printf("Loopback test: Send data and it will be echoed back\\n");
        printf("Press Ctrl+C to stop\\n\\n");
        
        uint8_t rx_byte;
        while (1) {
            if (HAL_UART_Receive(huart, &rx_byte, 1, 100) == HAL_OK) {
                HAL_UART_Transmit(huart, &rx_byte, 1, 100);
                printf("%c", rx_byte);
                fflush(stdout);
            }
        }
        
    } else {
        printf("Error: Invalid mode '%s'\\n", mode);
        print_usage(argv[0]);
        return 1;
    }

    /* Deinitialize */
    HAL_UART_DeInit(huart);
    
    return 0;
}
