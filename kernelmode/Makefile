CC=gcc
MODCFLAGS := -Wall -DMODULE -DMODVERSIONS -D__KERNEL__ -DLINUX -O2 -DDEBUG

# If your line is using DMT modulation, add this define and recompile
# MODFLAGS += -D_DMT_

ifndef KERNELDIR
  KERNELDIR=/lib/modules/`uname -r`/build
endif

INSTALLDIR=/lib/modules/`uname -r`/misc

INCDIR := -I$(KERNELDIR)/include  -include $(KERNELDIR)/include/linux/modversions.h

all: atmtotap eci.o

eci.o: eci.c usb_packets.h
	$(CC) $(MODCFLAGS) -c $(INCDIR) eci.c

install:
	@mkdir -p $(INSTALLDIR)
	cp eci.o  $(INSTALLDIR)
	depmod -a 

atmtotap: atmtotap.c
	$(CC) -lpthread -latm -o atmtotap atmtotap.c

clean:
	rm -f atmtotap *.o *.so
