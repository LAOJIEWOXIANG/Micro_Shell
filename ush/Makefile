CFLAGS=-g -Wall
CC=gcc

%.o : %.c
	$(CC) $(CFLAGS) -c $<

# all: ush expand

ush: ush.o expand.o builtin.o
	$(CC) $(CFLAGS) -o ush ush.o expand.o builtin.o

run:
	./ush

# dependencies
ush.o expand.o builtin.o: defn.h

clean:
	rm -f *.o *.out ush expand builtin