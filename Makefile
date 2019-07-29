CC=gcc
LIBS=`curl-config --libs` `pkg-config --libs json-c`
CFLAGS=-march=native -O2 `pkg-config --cflags json-c` -std=gnu17 -Wno-discarded-qualifiers -fopenmp

all:
	$(CC) $(LIBS) $(CFLAGS) *.c utron/*.c -o carpooling

clean:
	rm -f carpooling
