CC=gcc
MODCFLAGS := -Wall -DMODULE -DMODVERSIONS -D__KERNEL__ -DLINUX -D_DMT_ -O2 

INCDIR := /usr/src/linux/include

all: pppoatm.so eci.o

eci.o: eci.c usb_packets.h
	$(CC) $(MODCFLAGS) -c -I/usr/src/linux/include eci.c 
install:
	cp eci.o /lib/modules/`uname -r `/kernel/drivers/usb
	chown root:root /lib/modules/`uname -r `/kernel/drivers/usb
	depmod -a 
pppoatm.o:pppoatm.c
	$(CC) -I$(INCDIR) -c -g -O pppoatm.c

pppoatm.so:pppoatm.o
	$(CC) -shared -a -o pppoatm.so pppoatm.o -latm -fPIC

atmtotap: atmtotap.c
	$(CC) -lpthread -latm -o atmtotap atmtotap.c
