PREFIX ?= /usr/local

BIN = xkblayout-remember
SRC = xkblayout-remember.c

.PHONY: all install uninstall clean
.POSIX:

all : $(BIN)

install : $(BIN)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin/$(BIN)
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(BIN)

uninstall :
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)

clean :
	rm -f $(BIN)

$(BIN) : $(SRC)
	$(CC) $(SRC) -o $(BIN) -Wall -Wextra -Wno-parentheses \
		-O2 -lX11 $(shell pkg-config --cflags --libs glib-2.0)
