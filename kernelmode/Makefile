CC=gcc
MODCFLAGS := -Wall -DMODULE -D__KERNEL__ -DLINUX -O2
eci.o: eci.c usb_packets.h
	$(CC) $(MODCFLAGS) -c -I/usr/src/linux/include eci.c 
install:
	cp eci.o /lib/modules/`uname -r `/kernel/drivers/usb
	chown root:root /lib/modules/`uname -r `/kernel/drivers/usb
	depmod -a 
pppoatm.o:pppoatm.c
	$(CC) -c -O pppoatm.c

pppoatm.so:pppoatm.c
	$(CC) -shared -a pppoatm.so pppoatm.o
pppoatm:pppoatm.so
