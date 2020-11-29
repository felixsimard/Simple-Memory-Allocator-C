# .DEFAULT_GOAL=all

CC=gcc
CFLAGS=-fsanitize=signed-integer-overflow -fsanitize=undefined -g -std=gnu99 -O2 -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow

SMA=sma


sma: a3_test.c sma.c
	$(CC) -o $(SMA) $(CFLAGS) a3_test.c sma.c

clean:
	rm -rf $(SMA)
