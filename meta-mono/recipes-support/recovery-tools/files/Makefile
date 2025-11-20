CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -O2 -std=c99
LDFLAGS = -lhal_stm32f4

TARGETS = flash-program memory-test system-info

all: $(TARGETS)

flash-program: flash-program.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

memory-test: memory-test.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

system-info: system-info.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
