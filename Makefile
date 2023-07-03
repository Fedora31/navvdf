.PHONY: test

CC=gcc
SRC=src/*.c
CFLAGS=-Wall -Wextra -Iinclude -std=c89 -pedantic

all:
	$(CC) $(CFLAGS) $(SRC) -c
	ar -rcs libnavvdf.a *.o

test:
	$(CC) $(CFLAGS) test.c -c
	$(CC) test.o -o test -L. -lnavvdf -g

clean:
	rm *.o
