CC=gcc
CFLAGS += -I/usr/local/include
LDLIBS += -L/usr/local/lib -lpusb

default: pusb-view

clean:
	$(RM) pusb-view pusb-view.o

distclean: clean
	autoconf
	$(RM) -r Makefile configure config.* autom4te.cache
