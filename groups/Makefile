TARGETS = janus-groups

CFLAGS += -Wall -O6 -D_GNU_SOURCE

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS)

PREFIX ?= /usr/local

install: $(TARGETS)
	install -D -d $(DESTDIR)/$(PREFIX)/bin
	install -s -m 0755 $^ $(DESTDIR)/$(PREFIX)/bin

janus-groups: callout.o
janus-groups: inet.o ipv4-expand.o address.o node.o group.o
janus-groups: CFLAGS  += `pkg-config libipset --cflags`
janus-groups: LDFLAGS += `pkg-config libipset --libs`
