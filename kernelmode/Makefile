CC=gcc
MODCFLAGS := -Wall -DMODULE -DMODVERSIONS -D__KERNEL__ -DLINUX -D_DMT_ -O2 -DDEBUG

INCDIR := -I/usr/src/linux/include -I/usr/src/linux/include/asm/mach-default/

all: atmtotap eci.o

eci.o: eci.c usb_packets.h
	$(CC) $(MODCFLAGS) -c $(INCDIR) eci.c

install:
	cp eci.o /lib/modules/`uname -r `/kernel/drivers/usb
	chown root:root /lib/modules/`uname -r `/kernel/drivers/usb
	depmod -a 

atmtotap: atmtotap.c
	$(CC) -lpthread -latm -o atmtotap atmtotap.c

clean:
	rm -f atmtotap *.o *.so
