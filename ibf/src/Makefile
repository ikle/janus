TARGETS += ibf-login ibf-gate ibf-init

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS)

PREFIX ?= /usr/local

install: $(TARGETS)
	install -D -d $(DESTDIR)/$(PREFIX)/sbin
	install -s -m 0755 $^ $(DESTDIR)/$(PREFIX)/sbin

cgi.o conf.o: CFLAGS  += `pkg-config --cflags glib-2.0`

login-page.o: login-head login-tail avatar.png
	$(LD) -r -b binary -o $@ $^

chain-hash.o: CFLAGS += `pkg-config openssl --cflags`

ibf-login: CFLAGS  += `pkg-config --cflags libipset glib-2.0`
ibf-login: LDFLAGS += `pkg-config --libs   libipset glib-2.0 openssl` -lfcgi -lpam
ibf-login: chain-hash.o

ibf-gate: CFLAGS  += `pkg-config --cflags glib-2.0`
ibf-gate: LDFLAGS += `pkg-config --libs   glib-2.0` -lfcgi

ibf-init: LDFLAGS += `pkg-config --libs   glib-2.0`

ibf-login: conf.o cgi.o pam.o login-page.o
ibf-gate: cgi.o
ibf-init: conf.o service.o
