/*
 * GPIO Set Utility for STM32F411 with HAL
 * Sets GPIO pin to HIGH or LOW state
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"

void print_usage(const char *prog) {
    printf("Usage: %s <port> <pin> <state>\n", prog);
    printf("  port: A, B, C, D, E, or H\n");
    printf("  pin: 0-15\n");
    printf("  state: 0 (LOW), 1 (HIGH), or read\n");
    printf("\nExamples:\n");
    printf("  %s C 13 1    # Set PC13 to HIGH\n", prog);
    printf("  %s A 5 0     # Set PA5 to LOW\n", prog);
    printf("  %s B 7 read  # Read PB7 state\n", prog);
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

void GPIO_Init_Output(GPIO_TypeDef *gpio, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = (1 << pin);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

void GPIO_Init_Input(GPIO_TypeDef *gpio, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = (1 << pin);
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }

    char port = argv[1][0];
    int pin = atoi(argv[2]);
    char *state_str = argv[3];

    if (pin < 0 || pin > 15) {
        printf("Error: Pin must be between 0 and 15\n");
        return 1;
    }

    GPIO_TypeDef *gpio = get_gpio_port(port);
    if (!gpio) {
        printf("Error: Invalid port '%c'\n", port);
        return 1;
    }

    /* Initialize HAL */
    HAL_Init();

    /* Check if read operation */
    if (strcmp(state_str, "read") == 0 || strcmp(state_str, "READ") == 0) {
        /* Configure as input */
        GPIO_Init_Input(gpio, pin);
        
        /* Read state */
        GPIO_PinState state = HAL_GPIO_ReadPin(gpio, (1 << pin));
        printf("GPIO%c.%d = %s\n", port, pin, state ? "HIGH" : "LOW");
        
        HAL_GPIO_DeInit(gpio, (1 << pin));
        return 0;
    }

    /* Parse state */
    int state = atoi(state_str);
    if (state != 0 && state != 1) {
        printf("Error: State must be 0, 1, or 'read'\n");
        return 1;
    }

    /* Configure pin as output */
    GPIO_Init_Output(gpio, pin);

    /* Set the pin state */
    HAL_GPIO_WritePin(gpio, (1 << pin), state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    printf("GPIO%c.%d set to %s\n", port, pin, state ? "HIGH" : "LOW");

    /* Deinitialize */
    HAL_GPIO_DeInit(gpio, (1 << pin));
    
    return 0;
}
