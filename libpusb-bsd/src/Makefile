# Author : Benoit PAPILLAULT <benoit.papillault@free.fr>
# Creation : 07/10/2003

prefix = /usr/local/
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include

CC=gcc
RM=rm -f
PACKAGE_VERSION=0.1

CFLAGS  += -Wall -O2
LDFLAGS += -s

PUSB_SRC = pusb.c pusb-common.c
PUSB_OBJ = $(PUSB_SRC:.c=.o)

DEP = Makefile.dep

default: libpusb.so

libpusb.so: $(PUSB_OBJ)
	$(CC) -shared -o $@ $^

clean:
	$(RM) pusb.o pusb-common.o libpusb.so $(DEP)

$(DEP): $(PUSB_SRC)
	$(CC) $(CFLAGS) -MM $^ > $@ -include $(DEP)

install: libpusb.so pusb.h
	install -d $(includedir)
	install pusb.h $(includedir)
	install -d $(libdir)
	install libpusb.so $(libdir)/libpusb.so.$(PACKAGE_VERSION)
	ln -sf libpusb.so.$(PACKAGE_VERSION) $(libdir)/libpusb.so


