CC=gcc -g

benchmark.o: benchmark.c library.h library.c
	rm ../ramdisk/map
	$(CC) library.c benchmark.c -o benchmark.o
       
clean:
	rm *.o
