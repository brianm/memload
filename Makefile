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

test: build
	memcached -p 11311 -d -P /tmp/memload_test_1
	memcached -p 11411 -d -P /tmp/memload_test_2
	./$(BINARY) -s localhost:11311 -s localhost:11411 -v -d ',' ./test.csv
	kill `cat /tmp/memload_test_1` `cat /tmp/memload_test_2`
	rm /tmp/memload_test_*