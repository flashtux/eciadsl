
CFLAGS += -Wall -g
LDFLAGS += -g -Wall

# Final targets
BIN = eci-load1 eci-load2 pppoeci

all: ${BIN}

eci-load1: eci-load1.o pusb.o
eci-load2: eci-load2.o pusb.o
pppoeci: pppoeci.o pusb.o


pusb.o : pusb.c pusb-linux.c pusb.h
eci-load2.o : eci-load2.c pusb.h
pppoa2.o: pppoa2.c pusb.h pool.h modem.h

install: all
	cp eci-load1 /usr/local/bin
	chmod 500 /usr/local/bin/eci-load1
	cp eci-load2 /usr/local/bin
	chmod 500 /usr/local/bin/eci-load2
	cp eci_firm_kit_wanadoo.bin /usr/local/bin
	chmod 500 /usr/local/bin/eci_firm_kit_wanadoo.bin
	cp eci_wan3.bin /usr/local/bin/eci_wan3.bin
	chmod 500 /usr/local/bin/eci_wan3.bin
	cp pppoeci /usr/local/bin
	chmod 500 /usr/local/bin/pppoeci
	chown root:root /usr/local/bin/pppoeci
	cp startmodem /usr/local/bin
	chmod 500 /usr/local/bin/startmodem 
	chown root:root /usr/local/bin/startmodem
	cp adsl /etc/ppp/peers/
	chmod 500 /etc/ppp/peers/adsl
	chown root:root /etc/ppp/peers/adsl
	echo "dont forget to edit adsl and chap-secrets files\n"

clean:
	rm -f ${BIN} *.o 2>/dev/null
