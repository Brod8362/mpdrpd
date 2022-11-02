CC=gcc
CFLAGS=-g3 -Wall -Wextra -Wpedantic
LIBS=-lmpdclient

libmpd_test: test.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^