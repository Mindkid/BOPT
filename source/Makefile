CC=gcc
CFLAGS=-L/usr/local/lib -I/usr/local/include -lpthread

art.o: art.c
	$(CC) art.c -o art.o

main.o: main.c art.0
	$(CC) main.c -o main.o $(CFLAGS)
       
clean:
	rm *.o
