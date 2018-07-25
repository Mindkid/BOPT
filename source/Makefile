CC=gcc -g
CFLAGS=-lpthread

main.o: main.c list.c BOPL.c
	$(CC)  BOPL.c list.c main.c -o main.o $(CFLAGS)
       
clean:
	rm *.o
