CC=gcc -g
CFLAGS=-lpthread

main.o: main.c list.c BOPL.c hashmap.c
	$(CC)  BOPL.c list.c  hashmap.c main.c -o main.o $(CFLAGS)
       
clean:
	rm *.o
	rm ../ramdisk/*.dat
