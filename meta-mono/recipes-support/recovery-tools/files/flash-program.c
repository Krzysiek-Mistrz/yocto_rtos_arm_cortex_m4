/*
 * Flash Programming Utility for STM32F411 with HAL
 * Programs internal flash memory
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"

void print_usage(const char *prog) {
    printf("Usage: %s <command> [args]\n", prog);
    printf("Commands:\n");
    printf("  erase <sector>     - Erase flash sector (0-7)\n");
    printf("  write <addr> <data> - Write data to address (hex)\n");
    printf("  read <addr> <len>  - Read data from address\n");
    printf("  info               - Display flash information\n");
    printf("\nExamples:\n");
    printf("  %s erase 5\n", prog);
    printf("  %s write 0x08010000 0xDEADBEEF\n", prog);
    printf("  %s read 0x08000000 256\n", prog);
}

void flash_info(void) {
    printf("STM32F411 Flash Information:\n");
    printf("  Total Size: 512 KB\n");
    printf("  Sectors: 8\n");
    printf("  Base Address: 0x08000000\n");
    printf("\nSector Layout:\n");
    printf("  Sector 0: 0x08000000 - 0x08003FFF (16 KB)\n");
    printf("  Sector 1: 0x08004000 - 0x08007FFF (16 KB)\n");
    printf("  Sector 2: 0x08008000 - 0x0800BFFF (16 KB)\n");
    printf("  Sector 3: 0x0800C000 - 0x0800FFFF (16 KB)\n");
    printf("  Sector 4: 0x08010000 - 0x0801FFFF (64 KB)\n");
    printf("  Sector 5: 0x08020000 - 0x0803FFFF (128 KB)\n");
    printf("  Sector 6: 0x08040000 - 0x0805FFFF (128 KB)\n");
    printf("  Sector 7: 0x08060000 - 0x0807FFFF (128 KB)\n");
}

int flash_erase_sector(uint32_t sector) {
    if (sector > 7) {
        printf("Error: Invalid sector %u (must be 0-7)\n", sector);
        return -1;
    }

    printf("Erasing sector %u...\n", sector);

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError = 0;

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector = sector;
    EraseInitStruct.NbSectors = 1;

    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);

    HAL_FLASH_Lock();

    if (status != HAL_OK) {
        printf("Error: Flash erase failed (status: %d, error: %u)\n", status, SectorError);
        return -1;
    }

    printf("Sector %u erased successfully!\n", sector);
    return 0;
}

int flash_write(uint32_t address, uint32_t data) {
    if (address < 0x08000000 || address >= 0x08080000) {
        printf("Error: Address 0x%08X out of flash range\n", address);
        return -1;
    }

    printf("Writing 0x%08X to address 0x%08X...\n", data, address);

    HAL_FLASH_Unlock();

    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);

    HAL_FLASH_Lock();

    if (status != HAL_OK) {
        printf("Error: Flash write failed (status: %d)\n", status);
        return -1;
    }

    /* Verify */
    uint32_t read_data = *(volatile uint32_t*)address;
    if (read_data != data) {
        printf("Error: Verification failed! Read: 0x%08X\n", read_data);
        return -1;
    }

    printf("Write successful and verified!\n");
    return 0;
}

void flash_read(uint32_t address, uint32_t length) {
    if (address < 0x08000000 || address >= 0x08080000) {
        printf("Error: Address 0x%08X out of flash range\n", address);
        return;
    }

    printf("Reading %u bytes from address 0x%08X:\n", length, address);

    uint8_t *ptr = (uint8_t*)address;
    for (uint32_t i = 0; i < length; i++) {
        if (i % 16 == 0) {
            printf("\n0x%08X: ", address + i);
        }
        printf("%02X ", ptr[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    HAL_Init();

    char *command = argv[1];

    if (strcmp(command, "info") == 0) {
        flash_info();
    } else if (strcmp(command, "erase") == 0) {
        if (argc < 3) {
            printf("Error: Missing sector number\n");
            return 1;
        }
        uint32_t sector = atoi(argv[2]);
        return flash_erase_sector(sector);
    } else if (strcmp(command, "write") == 0) {
        if (argc < 4) {
            printf("Error: Missing address or data\n");
            return 1;
        }
        uint32_t address = strtoul(argv[2], NULL, 16);
        uint32_t data = strtoul(argv[3], NULL, 16);
        return flash_write(address, data);
    } else if (strcmp(command, "read") == 0) {
        if (argc < 4) {
            printf("Error: Missing address or length\n");
            return 1;
        }
        uint32_t address = strtoul(argv[2], NULL, 16);
        uint32_t length = atoi(argv[3]);
        flash_read(address, length);
    } else {
        printf("Error: Unknown command '%s'\n", command);
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
