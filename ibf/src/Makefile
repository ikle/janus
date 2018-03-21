TARGETS += ibf-login ibf-gate

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS)

PREFIX ?= /usr/local

install: $(TARGETS)
	install -D -d $(DESTDIR)/$(PREFIX)/sbin
	install -s -m 0755 $^ $(DESTDIR)/$(PREFIX)/sbin
	chmod u+s $(DESTDIR)/$(PREFIX)/sbin/ibf-login

cgi.o conf.o: CFLAGS  += `pkg-config --cflags glib-2.0`

login-page.o: login-head login-tail avatar.png
	$(LD) -r -b binary -o $@ $^

ibf-login: LDFLAGS += -lpam
ibf-login ibf-gate: CFLAGS  += `pkg-config --cflags libipset glib-2.0`
ibf-login ibf-gate: LDFLAGS += `pkg-config --libs   libipset glib-2.0` -lfcgi

ibf-login: conf.o cgi.o pam.o login-page.o
ibf-gate: cgi.o
