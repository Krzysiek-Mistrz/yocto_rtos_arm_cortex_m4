CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -O2 -std=c99
LDFLAGS = -lhal_stm32f4

TARGETS = i2c-scan i2c-read i2c-write spi-test uart-test

all: $(TARGETS)

i2c-scan: i2c-scan.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

i2c-read: i2c-read.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

i2c-write: i2c-write.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

spi-test: spi-test.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

uart-test: uart-test.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
