CC=gcc
CFLAGS=-g3 -Wall -Wextra -Wpedantic
LIBS=-lmpdclient -ldiscord-rpc

mpdrpd: mpdrpd.c discord.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

clean:
	rm -f mpdrpd

.PHONY: clean