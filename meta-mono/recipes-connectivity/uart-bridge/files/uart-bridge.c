/**
 * @file uart-bridge.c
 * @brief UART bridge daemon for i.MX6ULL <-> STM32F411 communication
 * 
 * This daemon runs on i.MX6ULL Linux and manages communication with
 * STM32F411 running Zephyr RTOS via UART2 (ttymxc1).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <stdbool.h>

#include "uart-protocol.h"


#define UNIX_SOCKET_PATH "/var/run/uart-bridge.sock"
#define MAX_CLIENTS 5
#define READ_TIMEOUT_SEC 5
static int uart_fd = -1;
static int socket_fd = -1;
static volatile bool running = true;
static int open_uart(const char *device, speed_t baudrate);
static int create_unix_socket(const char *path);
static void signal_handler(int signum);
static void process_uart_data(void);
static void process_client_data(int client_fd);
static int send_to_stm32(const char *message);
static void cleanup(void);

/**
 * @brief Open and configure UART device
 */
static int open_uart(const char *device, speed_t baudrate) {
    int fd;
    struct termios tty;

    fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        syslog(LOG_ERR, "Failed to open UART device %s: %s", device, strerror(errno));
        return -1;
    }

    if (tcgetattr(fd, &tty) != 0) {
        syslog(LOG_ERR, "Failed to get UART attributes: %s", strerror(errno));
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, baudrate);
    cfsetispeed(&tty, baudrate);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     /* 8-bit chars */
    tty.c_iflag &= ~IGNBRK;                         /* Disable break processing */
    tty.c_lflag = 0;                                /* No signaling chars, no echo, no canonical processing */
    tty.c_oflag = 0;                                /* No remapping, no delays */
    tty.c_cc[VMIN]  = 0;                            /* Read doesn't block */
    tty.c_cc[VTIME] = 5;                            /* 0.5 seconds read timeout */

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);         /* Shut off xon/xoff ctrl */
    tty.c_cflag |= (CLOCAL | CREAD);                /* Ignore modem controls, enable reading */
    tty.c_cflag &= ~(PARENB | PARODD);              /* No parity */
    tty.c_cflag &= ~CSTOPB;                         /* 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;                        /* No hardware flow control */

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        syslog(LOG_ERR, "Failed to set UART attributes: %s", strerror(errno));
        close(fd);
        return -1;
    }

    syslog(LOG_INFO, "UART device %s opened successfully", device);
    return fd;
}

/**
 * @brief Create Unix domain socket for local IPC
 */
static int create_unix_socket(const char *path) {
    int fd;
    struct sockaddr_un addr;

    unlink(path);

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        syslog(LOG_ERR, "Failed to create socket: %s", strerror(errno));
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        syslog(LOG_ERR, "Failed to bind socket: %s", strerror(errno));
        close(fd);
        return -1;
    }

    if (listen(fd, MAX_CLIENTS) < 0) {
        syslog(LOG_ERR, "Failed to listen on socket: %s", strerror(errno));
        close(fd);
        return -1;
    }

    syslog(LOG_INFO, "Unix socket created at %s", path);
    return fd;
}

/**
 * @brief Signal handler for graceful shutdown
 */
static void signal_handler(int signum) {
    syslog(LOG_INFO, "Received signal %d, shutting down...", signum);
    running = false;
}

/**
 * @brief Send message to STM32 via UART
 */
static int send_to_stm32(const char *message) {
    char buffer[MAX_MESSAGE_LENGTH];
    int len;

    len = snprintf(buffer, sizeof(buffer), "%s\n", message);
    if (len < 0 || len >= sizeof(buffer)) {
        syslog(LOG_ERR, "Message too long");
        return -1;
    }

    int written = write(uart_fd, buffer, len);
    if (written != len) {
        syslog(LOG_ERR, "Failed to write to UART: %s", strerror(errno));
        return -1;
    }

    syslog(LOG_DEBUG, "Sent to STM32: %s", message);
    return 0;
}

/**
 * @brief Process data received from UART (STM32)
 */
static void process_uart_data(void) {
    static char buffer[MAX_MESSAGE_LENGTH];
    static int buffer_pos = 0;
    char read_buf[64];
    int n;

    n = read(uart_fd, read_buf, sizeof(read_buf) - 1);
    if (n > 0) {
        read_buf[n] = '\0';
        
        for (int i = 0; i < n; i++) {
            if (read_buf[i] == '\n') {
                buffer[buffer_pos] = '\0';
                syslog(LOG_DEBUG, "Received from STM32: %s", buffer);
                
                /* TODO: Forward to connected clients or process locally */
                printf("STM32: %s\n", buffer);
                
                buffer_pos = 0;
            } else if (buffer_pos < MAX_MESSAGE_LENGTH - 1) {
                buffer[buffer_pos++] = read_buf[i];
            } else {
                syslog(LOG_WARNING, "Buffer overflow, resetting");
                buffer_pos = 0;
            }
        }
    }
}

/**
 * @brief Process data from client socket
 */
static void process_client_data(int client_fd) {
    char buffer[MAX_MESSAGE_LENGTH];
    int n;

    n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        
        if (buffer[n-1] == '\n') {
            buffer[n-1] = '\0';
        }
        
        syslog(LOG_DEBUG, "Received from client: %s", buffer);
        
        send_to_stm32(buffer);
    }
}

/**
 * @brief Cleanup resources
 */
static void cleanup(void) {
    if (uart_fd >= 0) {
        close(uart_fd);
        uart_fd = -1;
    }
    
    if (socket_fd >= 0) {
        close(socket_fd);
        socket_fd = -1;
    }
    
    unlink(UNIX_SOCKET_PATH);
    syslog(LOG_INFO, "Cleanup completed");
}

/**
 * @brief Main function
 */
int main(int argc, char *argv[]) {
    fd_set read_fds;
    int max_fd;
    struct timeval timeout;
    int client_fd = -1;

    openlog("uart-bridge", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "UART Bridge Daemon starting...");

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    uart_fd = open_uart(UART_DEVICE, B115200);
    if (uart_fd < 0) {
        syslog(LOG_ERR, "Failed to open UART, exiting");
        return EXIT_FAILURE;
    }

    socket_fd = create_unix_socket(UNIX_SOCKET_PATH);
    if (socket_fd < 0) {
        cleanup();
        return EXIT_FAILURE;
    }

    syslog(LOG_INFO, "UART Bridge Daemon running");

    while (running) {
        FD_ZERO(&read_fds);
        FD_SET(uart_fd, &read_fds);
        FD_SET(socket_fd, &read_fds);
        
        max_fd = (uart_fd > socket_fd) ? uart_fd : socket_fd;
        
        if (client_fd >= 0) {
            FD_SET(client_fd, &read_fds);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
        }

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ret = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            syslog(LOG_ERR, "select() failed: %s", strerror(errno));
            break;
        }

        if (ret == 0) {
            continue;
        }

        if (FD_ISSET(uart_fd, &read_fds)) {
            process_uart_data();
        }

        if (FD_ISSET(socket_fd, &read_fds)) {
            client_fd = accept(socket_fd, NULL, NULL);
            if (client_fd < 0) {
                syslog(LOG_ERR, "Failed to accept client: %s", strerror(errno));
            } else {
                syslog(LOG_INFO, "Client connected");
            }
        }

        if (client_fd >= 0 && FD_ISSET(client_fd, &read_fds)) {
            process_client_data(client_fd);
        }
    }

    cleanup();
    closelog();
    return EXIT_SUCCESS;
}
