CC=clang
LIBMEMCACHED_ROOT=/usr/local
PREFIX=/usr/local
BINARY=memload

build:
	$(CC) -o $(BINARY) -lmemcached -L$(LIBMEMCACHED_ROOT)/lib -I$(LIBMEMCACHED_ROOT)/include src/*.c

install: build
	cp $(BINARY) $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/$(BINARY)

clean:
	rm -f memload