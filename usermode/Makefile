# 23/12/2001 Benoit PAPILLAULT
#   make the file /etc/ppp/peers/adsl writable for user root (since most of
#   the time, he must change the "user").

# If the /etc/ppp/peers/adsl already exists, ask the user to overwrite it
# with the new one (but with keeping the same user)

# install: should check for "tty-ldisc-13" in /etc/modules.conf

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

install: all check-char-major-108 check-tty-ldisc-14 check-tty-ldisc-13 \
  install-etc-ppp-peers-adsl
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
	@echo "don't forget to edit /etc/ppp/peers/adsl and /etc/ppp/chap-secrets files"

clean:
	rm -f ${BIN} *.o 2>/dev/null

# for for the ppp_generic alias in /etc/modules.conf
check-char-major-108:
	@grep "^alias char-major-108" /etc/modules.conf > /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "adding ppp_generic alias ..." ; \
		echo "alias char-major-108 ppp_generic" >> /etc/modules.conf ; \
	fi

# check for ppp_synctty alias in /etc/modules.conf
check-tty-ldisc-14:
	@grep "^alias tty-ldisc-14" /etc/modules.conf > /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "adding ppp_synctty alias ..." ;\
		echo "alias tty-ldisc-14 ppp_synctty" >> /etc/modules.conf ; \
	fi

# check for the n_hdlc alias in /etc/modules.conf
check-tty-ldisc-13:
	@grep "^alias tty-ldisc-13" /etc/modules.conf > /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "adding n_hdlc alias ..." ;\
		echo "alias tty-ldisc-13 n_hdlc" >> /etc/modules.conf ; \
	fi

# install the /etc/ppp/peers/adsl file
install-etc-ppp-peers-adsl:
	@if [ -f /etc/ppp/peers/adsl ]; then \
		echo "modifying existing /etc/ppp/peers/adsl" ; \
		user=`grep "^user " /etc/ppp/peers/adsl` ; \
		grep -v "^user " adsl > /etc/ppp/peers/adsl ; \
		echo $$user >> /etc/ppp/peers/adsl ; \
	else \
		echo "creating /etc/ppp/peers/adsl" ; \
		cp adsl /etc/ppp/peers/adsl ; \
		chmod 700 /etc/ppp/peers/adsl ; \
		chown root:root /etc/ppp/peers/adsl ; \
	fi
uninstall:
	rm /usr/local/bin/eci-load1
	rm /usr/local/bin/eci-load2
	rm /usr/local/bin/eci_firm_kit_wanadoo.bin
	rm /usr/local/bin/eci_wan3.bin
	rm /usr/local/bin/pppoeci
	rm /usr/local/bin/startmodem 
	@echo "The driver has been successfully uninstalled"

