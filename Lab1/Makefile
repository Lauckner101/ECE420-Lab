CC = gcc
CFLAGS = -Wall -g

main: lab1_IO.o matrix_mult.o
	$(CC) $(CFLAGS) -o main lab1_IO.o matrix_mult.o

lab1_IO.o: lab1_IO.c
	$(CC) $(CFLAGS) -c lab1_IO.c

matrix_mult.o: matrix_mult.c
	$(CC) $(CFLAGS) -c matrix_mult.c


clean:
	del main.exe
	del lab1_IO.o
	del matrix_multi.o
