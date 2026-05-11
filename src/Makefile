CC = gcc
CFLAGS = -Wall -g -pthread


# Targets
all: server client

server: src/server/server.c
        $(CC) $(CFLAGS) -o server src/server/server.c


client: src/client/client.c
        $(CC) $(CFLAGS) -o client src/client/client.c


clean:
        rm -f server client server.exe client.exe

