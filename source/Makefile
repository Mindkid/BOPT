CC=gcc -D_FILE_OFFSET_BITS=64 -g -Wall
CFLAGS=-lpthread -lrt

all: boplFileReader benchmark boplFileReaderSTTRAM boplFileReaderOPTANE

boplFileReader: boplFileReader.c list.c hashmap.c log.c BOPL.c
	$(CC)  BOPL.c list.c  hashmap.c boplFileReader.c log.c -o  boplFileReader.o $(CFLAGS)

benchmark: benchmark.c stack.c
	$(CC)  benchmark.c stack.c -o  benchmark.o $(CFLAGS)

boplFileReaderSTTRAM: boplFileReader.c list.c hashmap.c log.c BOPL.c
	$(CC) -D __STT_RAM__ BOPL.c list.c  hashmap.c boplFileReader.c log.c -o boplFileReaderSTTRAM.o $(CFLAGS)

boplFileReaderOPTANE: boplFileReader.c list.c hashmap.c log.c BOPL.c
	$(CC) -D __OPTANE__ BOPL.c list.c  hashmap.c boplFileReader.c log.c -o boplFileReaderOPTANE.o $(CFLAGS)

clean:
	rm *.o
