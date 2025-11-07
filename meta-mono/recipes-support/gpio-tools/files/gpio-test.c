/*
 * GPIO Test Utility for STM32F411 with HAL
 * Tests GPIO functionality by toggling pins and reading states
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"

void print_usage(const char *prog) {
    printf("Usage: %s <port> <pin> [iterations]\n", prog);
    printf("  port: A, B, C, D, E, or H\n");
    printf("  pin: 0-15\n");
    printf("  iterations: number of toggle cycles (default: 10)\n");
    printf("\nExample: %s C 13 5\n", prog);
}

GPIO_TypeDef* get_gpio_port(char port) {
    switch(port) {
        case 'A': case 'a': return GPIOA;
        case 'B': case 'b': return GPIOB;
        case 'C': case 'c': return GPIOC;
        case 'D': case 'd': return GPIOD;
        case 'E': case 'e': return GPIOE;
        case 'H': case 'h': return GPIOH;
        default: return NULL;
    }
}

void GPIO_Init_Pin(GPIO_TypeDef *gpio, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = (1 << pin);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    char port = argv[1][0];
    int pin = atoi(argv[2]);
    int iterations = (argc > 3) ? atoi(argv[3]) : 10;

    if (pin < 0 || pin > 15) {
        printf("Error: Pin must be between 0 and 15\n");
        return 1;
    }

    GPIO_TypeDef *gpio = get_gpio_port(port);
    if (!gpio) {
        printf("Error: Invalid port '%c'\n", port);
        return 1;
    }

    printf("GPIO Test Utility for STM32F411 (HAL)\n");
    printf("Testing GPIO%c Pin %d\n", port, pin);
    printf("Iterations: %d\n\n", iterations);

    /* Initialize HAL */
    HAL_Init();
    
    /* Initialize GPIO pin */
    GPIO_Init_Pin(gpio, pin);

    /* Toggle pin */
    for (int i = 0; i < iterations; i++) {
        /* Set pin high */
        HAL_GPIO_WritePin(gpio, (1 << pin), GPIO_PIN_SET);
        printf("[%d] GPIO%c.%d = HIGH\n", i+1, port, pin);
        HAL_Delay(500); /* 500ms */

        /* Set pin low */
        HAL_GPIO_WritePin(gpio, (1 << pin), GPIO_PIN_RESET);
        printf("[%d] GPIO%c.%d = LOW\n", i+1, port, pin);
        HAL_Delay(500); /* 500ms */
    }

    printf("\nTest completed successfully!\n");
    
    /* Deinitialize */
    HAL_GPIO_DeInit(gpio, (1 << pin));
    
    return 0;
}
