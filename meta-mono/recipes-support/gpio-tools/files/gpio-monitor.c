/*
 * GPIO Monitor Utility for STM32F411 with HAL
 * Monitors GPIO pin states in real-time
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include "stm32f4xx_hal.h"

static volatile int keep_running = 1;

void signal_handler(int sig) {
    keep_running = 0;
}

void print_usage(const char *prog) {
    printf("Usage: %s <port> <pin> [interval_ms]\n", prog);
    printf("  port: A, B, C, D, E, or H\n");
    printf("  pin: 0-15\n");
    printf("  interval_ms: monitoring interval in milliseconds (default: 100)\n");
    printf("\nPress Ctrl+C to stop monitoring\n");
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

void GPIO_Init_Input(GPIO_TypeDef *gpio, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = (1 << pin);
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
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
    int interval_ms = (argc > 3) ? atoi(argv[3]) : 100;

    if (pin < 0 || pin > 15) {
        printf("Error: Pin must be between 0 and 15\n");
        return 1;
    }

    GPIO_TypeDef *gpio = get_gpio_port(port);
    if (!gpio) {
        printf("Error: Invalid port '%c'\n", port);
        return 1;
    }

    /* Set up signal handler for Ctrl+C */
    signal(SIGINT, signal_handler);

    printf("GPIO Monitor for STM32F411 (HAL)\n");
    printf("Monitoring GPIO%c Pin %d (interval: %dms)\n", port, pin, interval_ms);
    printf("Press Ctrl+C to stop\n\n");

    /* Initialize HAL */
    HAL_Init();
    
    /* Configure pin as input */
    GPIO_Init_Input(gpio, pin);

    GPIO_PinState last_state = GPIO_PIN_RESET;
    last_state = (GPIO_PinState)2; /* Invalid state to force first print */
    int count = 0;

    while (keep_running) {
        GPIO_PinState current_state = HAL_GPIO_ReadPin(gpio, (1 << pin));
        
        if (current_state != last_state) {
            printf("[%d] GPIO%c.%d changed to %s\n", 
                   count++, port, pin, current_state ? "HIGH" : "LOW");
            last_state = current_state;
        }

        HAL_Delay(interval_ms);
    }

    printf("\nMonitoring stopped.\n");
    
    /* Deinitialize */
    HAL_GPIO_DeInit(gpio, (1 << pin));
    
    return 0;
}
