NAME = libjanus.so

MAJOR   = 0
MINOR   = 0
RELEASE = 0

SONAME = $(NAME).$(MAJOR)
TARGET = $(SONAME).$(MINOR).$(RELEASE)

all: $(TARGET)

clean:
	rm -f *.o $(NAME)*

PREFIX ?= /usr/local

install: $(TARGET)
	install -D -d $(DESTDIR)/$(PREFIX)/lib
	install -s -m 0644 $^   $(DESTDIR)/$(PREFIX)/lib
	cp -a $(SONAME) $(NAME) $(DESTDIR)/$(PREFIX)/lib

	install -D -d $(DESTDIR)/$(PREFIX)/include
	install -m 0644 janus.h $(DESTDIR)/$(PREFIX)/include

$(TARGET): CFLAGS += -fPIC
$(TARGET): CFLAGS += `pkg-config glib-2.0 --cflags`

$(TARGET): LDFLAGS += -shared -Wl,-soname,$(SONAME)
$(TARGET): LDFLAGS += `pkg-config glib-2.0 --libs`

$(TARGET): node.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	ln -sf $(TARGET) $(SONAME)
	ln -sf $(SONAME) $(NAME)
