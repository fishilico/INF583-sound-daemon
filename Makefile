# Compiler
CC = gcc
CFLAGS = -Wall -pedantic -g -std=c99
LD = gcc
LDFLAGS = -Wall -pedantic -g -std=c99

# Recompile everything if headers change
HEADERS = daemon.h player.h
SOURCES = main.c daemon.c player.c
OBJS = $(SOURCES:%.c=%.o)
BIN = player

# Targets
TARGETS = $(BIN)

all: $(TARGETS)

clean:
	rm -f *~ a.out
	rm -f *.o

distclean: clean
	rm -f $(TARGETS) *.a *.so

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

