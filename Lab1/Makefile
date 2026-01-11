CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lpthread -lm

main: lab1_IO.o matrix_multi.o
	$(CC) $(CFLAGS) -o main lab1_IO.o matrix_multi.o $(LDFLAGS)

lab1_IO.o: lab1_IO.c
	$(CC) $(CFLAGS) -c lab1_IO.c

matrix_multi.o: matrix_multi.c
	$(CC) $(CFLAGS) -c matrix_multi.c


clean:
	del main.exe
	del lab1_IO.o
	del matrix_multi.o
