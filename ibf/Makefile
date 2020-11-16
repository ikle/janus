PREFIX ?= /usr/local

datadir ?= $(PREFIX)/share
cfgdir   = $(datadir)/edge/templates-cfg

.PHONY: all clean install

all:
	make -C src all

clean:
	make -C src clean

cpiop = find  . ! -regex '\(.*~\|.*\.bak\|.*\.swp\|.*\#.*\#\)' -print0 | \
  cpio -0pd

install: all
	make -C src install DESTDIR='$(DESTDIR)' PREFIX='$(PREFIX)'
	mkdir -p $(DESTDIR)$(cfgdir)
	cd templates-cfg; $(cpiop) $(DESTDIR)$(cfgdir)
