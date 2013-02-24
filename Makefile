# Compiler
CC = gcc
CFLAGS = -Wall -pedantic -g -std=c99
LD = gcc
LDFLAGS = -Wall -pedantic -g -std=c99

# Recompile everything if headers change
HEADERS = daemon.h

# Daemon files
DAEMON_SOURCES = main.c daemon.c player.c
DAEMON_OBJS = $(DAEMON_SOURCES:%.c=%.o)
DAEMON_BIN = playerd

# Targets
TARGETS = $(DAEMON_BIN)

all: $(TARGETS)

clean:
	rm -f *~ a.out
	rm -f *.o

distclean: clean
	rm -f $(TARGETS) *.a *.so

$(DAEMON_BIN): $(DAEMON_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

