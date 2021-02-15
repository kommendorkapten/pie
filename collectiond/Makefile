include ./../include.mk

sources=$(wildcard *.c)
objects=$(subst .c,.o,$(sources))
bin=collectiond

.PHONY: all clean build
all: build

$(bin): $(objects)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lwebsockets $(LCRYPTO) $(LIMG) -lsqlite3 $(LNET)

build: $(objects)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.a
