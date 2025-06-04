CC=gcc
CFLAGS=`pkg-config --cflags gtk+-3.0`
LIBS=`pkg-config --libs gtk+-3.0`

all: tvb-downloader

src/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o src/main.o

TVB_BIN=tvb-downloader

$(TVB_BIN): src/main.o
	$(CC) src/main.o -o $(TVB_BIN) $(LIBS)

clean:
	rm -f src/*.o $(TVB_BIN)
