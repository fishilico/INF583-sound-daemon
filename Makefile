CC      = gcc
CFLAGS  = -Wall -pedantic -g -std=c99
LD      = gcc
LDFLAGS = -Wall -pedantic -g -std=c99
SOURCES = main.c daemon.c player.c
HEADERS = daemon.h
OBJS    = $(SOURCES:%.c=%.o)
BIN     = playerd
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

