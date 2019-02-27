CC=gcc -g  -march=native

benchmark.o: benchmark.c library.h library.c
	rm -f ../ramdisk/*
	$(CC) library.c benchmark.c -o benchmark.o

clean:
	rm *.o
