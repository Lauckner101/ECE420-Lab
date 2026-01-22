CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lpthread -lm

main: server.o
	$(CC) $(CFLAGS) -o main server.o $(LDFLAGS)

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

clean:
	del main server.o