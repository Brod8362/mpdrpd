CC=gcc
CFLAGS=-g3 -Wall -Wextra -Wpedantic -std=gnu11
LIBS=-lmpdclient -ldiscord-rpc -pthread -Iinclude/

mpdrpd: src/mpdrpd.c src/discord.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

clean:
	rm -f mpdrpd

.PHONY: clean