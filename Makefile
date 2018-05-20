CC=gcc
CFLAGS=-Wall -g -std=c11
LDFLAGS=

PROGS=mlbf
SRCS=$(wildcard *.c)
HEADERS=$(wildcard *.h)
OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

all: $(PROGS)

mlbf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm *.o $(PROGS)

depend: .depend

.depend: $(HEADERS)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend;

include .depend
