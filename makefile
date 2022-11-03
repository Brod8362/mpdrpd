CC=gcc
CFLAGS=-g3 -Wall -Wextra -Wpedantic -std=gnu11
LIBS=-lmpdclient -ldiscord-rpc -pthread -Iinclude/

mpdrpd: src/mpdrpd.c src/discord.c src/log.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

clean:
	rm -f mpdrpd

install:
	install -D ./mpdrpd /usr/local/bin/mpdrpd
	install -D -m 644 ./mpdrpd.service /usr/lib/systemd/user/mpdrpd.service

.PHONY: clean