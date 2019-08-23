CC=gcc
LIBS=`curl-config --libs` `pkg-config --libs json-c`
CFLAGS=-O2 `pkg-config --cflags json-c` -Wno-discarded-qualifiers -fopenmp

all:
	$(CC) $(LIBS) $(CFLAGS) *.c utron/*.c -o carpooling

clean:
	rm -f carpooling
