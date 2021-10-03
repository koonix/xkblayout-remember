PREFIX ?= /usr/local

BIN = xkblayout-remember
SRC = xkblayout-remember.cpp

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
	g++ -Wall $(SRC) -o $(BIN) -lX11
