# $Id$

# For using pusb library :
CFLAGS += -I../../usermode

default: bin2hex ezusb-mem

bin2hex: bin2hex.o
ezusb-mem: ezusb-mem.o ../../usermode/pusb.o

clean:
	$(RM) bin2hex ezusb-mem bin2hex.o ezusb-mem.o
