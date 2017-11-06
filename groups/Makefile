TARGETS = callout.o inet.o ipv4-expand.o address.o node.o groups.o

CFLAGS += -Wall -O6 -D_GNU_SOURCE

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS)

PREFIX ?= /usr/local

install: $(TARGETS)
	install -D -d $(DESTDIR)/$(PREFIX)/bin
	install -s -m 0755 $^ $(DESTDIR)/$(PREFIX)/bin
