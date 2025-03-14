CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb
LIBS=-lm

geom: main.c
	$(CC) $(CFLAGS) -o geom main.c $(LIBS)

clean:
	rm -f geom