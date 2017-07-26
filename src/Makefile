TARGETS = item-test node-test conf-test type.o

CC ?= gcc
CFLAGS ?= -O3

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS)

PREFIX ?= /usr/local

install: $(TARGETS)
	install -D -d $(DESTDIR)/$(PREFIX)/bin
	install -s -m 0755 $^ $(DESTDIR)/$(PREFIX)/bin

item-test: item.o
node-test: node.o
conf-test: conf.o node.o item.o

test: conf-test
	./conf-test < test/conf.session
