CC=gcc -D_FILE_OFFSET_BITS=64 -g -Wall
CFLAGS=-lpthread -lrt

main.o: main.c list.c hashmap.c log.c BOPL.c
	$(CC)  BOPL.c list.c  hashmap.c main.c log.c -o  main.o $(CFLAGS)

fileReader: boplFileReader.c list.c hashmap.c log.c BOPL.c
	$(CC)  BOPL.c list.c  hashmap.c boplFileReader.c log.c -o  boplFileReader.o $(CFLAGS)

benchmark: benchmark.c list.c hashmap.c log.c BOPL.c stack.c
	$(CC) BOPL.c list.c  hashmap.c log.c benchmark.c stack.c -o  benchmark.o $(CFLAGS)


clean:
	rm *.o
	rm ../ramdisk/*.dat
