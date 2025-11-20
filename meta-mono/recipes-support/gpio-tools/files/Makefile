CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -O2 -std=c99
LDFLAGS = -lhal_stm32f4

TARGETS = gpio-test gpio-monitor gpio-set

all: $(TARGETS)

gpio-test: gpio-test.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

gpio-monitor: gpio-monitor.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

gpio-set: gpio-set.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
