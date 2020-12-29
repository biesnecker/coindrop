CC=gcc
ERRFLAGS=-Werror -Wall -Wextra -Wstrict-prototypes
LDLIBS=-lpthread
CFLAGS=-std=c17 -O0 -g -march=native $(ERRFLAGS)
BINARY_NAME=coindrop

OBJS = \
	main.o \
	mtrand.o \
	players.o

HEADERS = \
	board.h \
	mtrand.h \
	players.h \
	state.h

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BINARY_NAME): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)


.PHONY: clean
clean:
	find . -name '*.[oa]' -exec rm -f {} ';'
	rm -f $(BINARY_NAME)