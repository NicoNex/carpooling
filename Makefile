CC=gcc
LIBS=`curl-config --libs` `pkg-config --libs json-c` -lpthread
CFLAGS=-march=native -O2 `pkg-config --cflags json-c` -std=gnu17

all:
	$(CC) $(LIBS) $(CFLAGS) *.c utron/*.c -o carpooling

clean:
	rm -f carpooling
