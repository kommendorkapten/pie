include ./../include.mk

sources=$(wildcard *.c)
objects=$(subst .c,.o,$(sources))
lib=libvendor.a

.PHONY: all clean
all: $(objects)

lib: $(lib)

$(lib): $(objects)
	ar -cru $@ $(objects)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
