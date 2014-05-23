CC = gcc
CFLAGS = -Wall -c -g

connect-four: main.o
	$(CC) $^ -o $@

main.o: src/main.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f connect-four main.o
