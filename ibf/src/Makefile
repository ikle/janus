TARGETS += ibf-gate

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS)

PREFIX ?= /usr/local

install: $(TARGETS)
	install -D -d $(DESTDIR)/$(PREFIX)/bin
	install -s -m 0755 $^ $(DESTDIR)/$(PREFIX)/bin

cgi.o: CFLAGS  += `pkg-config --cflags glib-2.0`

ibf-gate: CFLAGS  += `pkg-config --cflags libipset glib-2.0`
ibf-gate: LDFLAGS += `pkg-config --libs   libipset glib-2.0` -lfcgi
ibf-gate: cgi.o
