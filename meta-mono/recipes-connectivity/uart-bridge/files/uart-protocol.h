/**
 * @file uart-protocol.h
 * @brief UART communication protocol between i.MX6ULL (Linux) and STM32F411 (Zephyr)
 * 
 * This header defines the protocol used for communication between:
 * - i.MX6ULL (Cortex-A7) running Linux
 * - STM32F411 (Cortex-M4) running Zephyr RTOS
 * 
 * Communication: UART at 115200 baud (configurable up to 3Mbps)
 * Format: ASCII text-based protocol with newline termination
 */

#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

/* Protocol version */
#define PROTOCOL_VERSION "1.0"

/* UART settings */
#define UART_BAUDRATE 115200
#define UART_DEVICE "/dev/ttymxc1"  /* UART2 on i.MX6ULL */

/* Message format */
#define MAX_MESSAGE_LENGTH 256
#define MESSAGE_DELIMITER '\n'
#define FIELD_SEPARATOR ':'
#define PARAM_SEPARATOR ','

/* Command types from Linux to STM32 */
#define CMD_GPIO_SET    "GPIO_SET"      /* Set GPIO pin: GPIO_SET:port,pin,value */
#define CMD_GPIO_GET    "GPIO_GET"      /* Get GPIO pin: GPIO_GET:port,pin */
#define CMD_I2C_READ    "I2C_READ"      /* Read I2C: I2C_READ:bus,addr,reg,len */
#define CMD_I2C_WRITE   "I2C_WRITE"     /* Write I2C: I2C_WRITE:bus,addr,reg,data */
#define CMD_ADC_READ    "ADC_READ"      /* Read ADC: ADC_READ:channel */
#define CMD_PWM_SET     "PWM_SET"       /* Set PWM: PWM_SET:channel,duty */
#define CMD_STATUS      "STATUS"        /* Get system status: STATUS */
#define CMD_PING        "PING"          /* Ping test: PING */
#define CMD_RESET       "RESET"         /* Reset STM32: RESET */

/* Response types from STM32 to Linux */
#define RESP_OK         "OK"            /* Success: OK or OK:data */
#define RESP_ERROR      "ERROR"         /* Error: ERROR:message */
#define RESP_STATUS     "STATUS"        /* Status: STATUS:{json} */
#define RESP_PONG       "PONG"          /* Ping response: PONG */

/* Error codes */
#define ERR_INVALID_CMD     "INVALID_COMMAND"
#define ERR_INVALID_PARAMS  "INVALID_PARAMETERS"
#define ERR_GPIO_FAIL       "GPIO_FAILED"
#define ERR_I2C_FAIL        "I2C_FAILED"
#define ERR_ADC_FAIL        "ADC_FAILED"
#define ERR_PWM_FAIL        "PWM_FAILED"
#define ERR_TIMEOUT         "TIMEOUT"
#define ERR_BUSY            "BUSY"

/* GPIO ports (STM32F411) */
#define GPIO_PORT_A 'A'
#define GPIO_PORT_B 'B'
#define GPIO_PORT_C 'C'
#define GPIO_PORT_D 'D'
#define GPIO_PORT_E 'E'
#define GPIO_PORT_H 'H'

/* Message structure */
typedef struct {
    char command[32];
    char params[MAX_MESSAGE_LENGTH - 32];
    char full_message[MAX_MESSAGE_LENGTH];
} uart_message_t;

/* Function prototypes for protocol handling */

/**
 * @brief Parse incoming UART message
 * @param raw_message Raw message string
 * @param msg Parsed message structure
 * @return true if parsing successful, false otherwise
 */
bool parse_message(const char *raw_message, uart_message_t *msg);

/**
 * @brief Build UART message from command and parameters
 * @param command Command string
 * @param params Parameters string (can be NULL)
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Number of bytes written, or -1 on error
 */
int build_message(const char *command, const char *params, char *buffer, size_t buffer_size);

/**
 * @brief Validate message format
 * @param message Message to validate
 * @return true if valid, false otherwise
 */
bool validate_message(const char *message);

#endif /* UART_PROTOCOL_H */
