/*
 * Memory Test Utility for STM32F411 with HAL
 * Tests RAM integrity
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"

#define SRAM_START 0x20000000
#define SRAM_SIZE  (128 * 1024)  /* 128 KB */

void print_usage(const char *prog) {
    printf("Usage: %s [test_type]\n", prog);
    printf("Test types:\n");
    printf("  quick  - Quick memory test (default)\n");
    printf("  full   - Full memory test (all patterns)\n");
    printf("  walk   - Walking bit test\n");
    printf("  info   - Display memory information\n");
}

void memory_info(void) {
    printf("STM32F411 Memory Information:\n");
    printf("  SRAM Start: 0x%08X\n", SRAM_START);
    printf("  SRAM Size: %u KB\n", SRAM_SIZE / 1024);
    printf("  SRAM End: 0x%08X\n", SRAM_START + SRAM_SIZE - 1);
    printf("  Flash Start: 0x08000000\n");
    printf("  Flash Size: 512 KB\n");
}

int test_pattern(uint32_t *start, uint32_t size, uint32_t pattern) {
    uint32_t words = size / 4;
    int errors = 0;

    /* Write pattern */
    for (uint32_t i = 0; i < words; i++) {
        start[i] = pattern;
    }

    /* Verify pattern */
    for (uint32_t i = 0; i < words; i++) {
        if (start[i] != pattern) {
            printf("  Error at 0x%08X: expected 0x%08X, got 0x%08X\n",
                   (uint32_t)&start[i], pattern, start[i]);
            errors++;
            if (errors >= 10) {
                printf("  Too many errors, stopping...\n");
                return errors;
            }
        }
    }

    return errors;
}

int quick_test(void) {
    printf("Running quick memory test...\n");
    
    /* Test a portion of RAM with basic patterns */
    uint32_t *test_area = (uint32_t*)(SRAM_START + 0x1000); /* Skip first 4KB */
    uint32_t test_size = 16 * 1024; /* Test 16KB */
    
    printf("Testing 0x%08X - 0x%08X\n", 
           (uint32_t)test_area, (uint32_t)test_area + test_size - 1);

    int total_errors = 0;

    printf("  Pattern 0x00000000... ");
    total_errors += test_pattern(test_area, test_size, 0x00000000);
    printf("%s\n", total_errors == 0 ? "PASS" : "FAIL");

    printf("  Pattern 0xFFFFFFFF... ");
    total_errors += test_pattern(test_area, test_size, 0xFFFFFFFF);
    printf("%s\n", total_errors == 0 ? "PASS" : "FAIL");

    printf("  Pattern 0xAAAAAAAA... ");
    total_errors += test_pattern(test_area, test_size, 0xAAAAAAAA);
    printf("%s\n", total_errors == 0 ? "PASS" : "FAIL");

    printf("  Pattern 0x55555555... ");
    total_errors += test_pattern(test_area, test_size, 0x55555555);
    printf("%s\n", total_errors == 0 ? "PASS" : "FAIL");

    return total_errors;
}

int full_test(void) {
    printf("Running full memory test...\n");
    printf("WARNING: This will take several minutes!\n");
    
    uint32_t *test_area = (uint32_t*)(SRAM_START + 0x1000);
    uint32_t test_size = SRAM_SIZE - 0x1000;
    
    printf("Testing 0x%08X - 0x%08X\n", 
           (uint32_t)test_area, (uint32_t)test_area + test_size - 1);

    int total_errors = 0;
    uint32_t patterns[] = {
        0x00000000, 0xFFFFFFFF, 0xAAAAAAAA, 0x55555555,
        0x12345678, 0x87654321, 0xDEADBEEF, 0xCAFEBABE
    };

    for (int i = 0; i < 8; i++) {
        printf("  Pattern 0x%08X... ", patterns[i]);
        int errors = test_pattern(test_area, test_size, patterns[i]);
        total_errors += errors;
        printf("%s\n", errors == 0 ? "PASS" : "FAIL");
    }

    return total_errors;
}

int walking_bit_test(void) {
    printf("Running walking bit test...\n");
    
    uint32_t *test_area = (uint32_t*)(SRAM_START + 0x1000);
    uint32_t test_size = 4 * 1024; /* Test 4KB */
    
    int total_errors = 0;

    for (int bit = 0; bit < 32; bit++) {
        uint32_t pattern = (1 << bit);
        printf("  Bit %d (0x%08X)... ", bit, pattern);
        int errors = test_pattern(test_area, test_size, pattern);
        total_errors += errors;
        printf("%s\n", errors == 0 ? "PASS" : "FAIL");
    }

    return total_errors;
}

int main(int argc, char *argv[]) {
    HAL_Init();

    char *test_type = (argc > 1) ? argv[1] : "quick";

    printf("STM32F411 Memory Test Utility (HAL)\n\n");

    int errors = 0;

    if (strcmp(test_type, "info") == 0) {
        memory_info();
    } else if (strcmp(test_type, "quick") == 0) {
        errors = quick_test();
    } else if (strcmp(test_type, "full") == 0) {
        errors = full_test();
    } else if (strcmp(test_type, "walk") == 0) {
        errors = walking_bit_test();
    } else {
        printf("Error: Unknown test type '%s'\n", test_type);
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(test_type, "info") != 0) {
        printf("\nTest completed with %d error(s)\n", errors);
        return (errors > 0) ? 1 : 0;
    }

    return 0;
}
