CC=gcc
LDFLAGS=-lpthread

all: client server

client: client.c
	$(CC) client.c -o client $(LDFLAGS)

server: server.c
	$(CC) server.c -o server $(LDFLAGS)

clean:
	rm -f client server
