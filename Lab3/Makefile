CC = gcc
CFLAGS = -Wall -g -fopenmp
LDFLAGS = -fopenmp -lm

main: main.o Lab3IO.o
	$(CC) $(CFLAGS) -o main main.o Lab3IO.o $(LDFLAGS)

datagen: datagen.o Lab3IO.o
	$(CC) $(CFLAGS) -o datagen datagen.o Lab3IO.o $(LDFLAGS)

main.o: main.c Lab3IO.h timer.h
	$(CC) $(CFLAGS) -c main.c

datagen.o: datagen.c Lab3IO.h
	$(CC) $(CFLAGS) -c datagen.c

Lab3IO.o: Lab3IO.c Lab3IO.h
	$(CC) $(CFLAGS) -c Lab3IO.c

clean:
	rm -f main datagen *.o data_input data_output
