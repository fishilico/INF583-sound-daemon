# Compiler
CC = gcc
CFLAGS = -Wall -pedantic -g -std=c99
LD = gcc
LDFLAGS = -Wall -pedantic -g -std=c99 -lpthread

# Recompile everything if headers change
HEADERS = daemon.h player.h
SOURCES = main.c daemon.c player.c
OBJS = $(SOURCES:%.c=%.o)
BIN = player
PACKAGE = player-iooss
PACKAGE_FILES = $(SOURCES) $(HEADERS) Makefile start-player.sh


# Targets
TARGETS = $(BIN)

all: $(TARGETS)

clean:
	rm -f *~ a.out
	rm -f *.o

distclean: clean
	rm -f $(TARGETS) *.a *.so

package: $(PACKAGE_FILES)
	! [ -d $(PACKAGE) ] || rmdir $(PACKAGE)
	mkdir $(PACKAGE)
	ln -v $^ $(PACKAGE)
	! [ -f $(PACKAGE).tar.gz ] || rm $(PACKAGE).tar.gz
	tar -czf $(PACKAGE).tar.gz $(PACKAGE)
	rm -r $(PACKAGE)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean distclean package
