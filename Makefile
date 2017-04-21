CC=gcc
CFLAGS=-Wall -g -std=c11
LDFLAGS=

PROGS=mlbf
MLBF_OBJS=mlbf.o interpreter.o

all: $(PROGS)

mlbf: $(MLBF_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm *.o $(PROGS)
