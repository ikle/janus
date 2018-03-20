PREFIX ?= /usr/local

sbindir ?= $(PREFIX)/sbin
datadir ?= $(PREFIX)/share
cfgdir   = $(datadir)/avadata/ibf/templates-cfg

.PHONY: all clean install

all:
	make -C src all

clean:
	make -C src clean

cpiop = find  . ! -regex '\(.*~\|.*\.bak\|.*\.swp\|.*\#.*\#\)' -print0 | \
  cpio -0pd

install: all
	mkdir -p $(DESTDIR)$(sbindir)
	install -m 00755 src/ibf-gate  $(DESTDIR)$(sbindir)
	install -m 04755 src/ibf-login $(DESTDIR)$(sbindir)
	mkdir -p $(DESTDIR)$(cfgdir)
	cd templates-cfg; $(cpiop) $(DESTDIR)$(cfgdir)
