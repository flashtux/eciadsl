# 23/12/2001 Benoit PAPILLAULT
#   make the file /etc/ppp/peers/adsl writable for user root (since most of
#   the time, he must change the "user").

# TODO: If the /etc/ppp/peers/adsl already exists, ask the user to overwrite it
# with the new one (but with keeping the same user and the persist option)

# install: should check for "tty-ldisc-13" in /etc/modules.conf. DONE.

# 15/02/2002 suggestion from Dumas Patrice <dumas@centre-cired.fr>
#   added ETCDIR, prefix and binprefix

CFLAGS += -Wall -g
LDFLAGS += -g -Wall -lpthread

prefix=/home/papillau/usr/local
binprefix=$(prefix)/bin
ETCDIR=/home/papillau/etc

# Final targets
BIN = eci-load1 eci-load2 pppoeci check-hdlc check-hdlc-bug

all: ${BIN}

eci-load1: eci-load1.o pusb.o
eci-load2: eci-load2.o pusb.o
pppoeci: pppoeci.o pusb.o
check-hdlc: check-hdlc.o
check-hdlc-bug: check-hdlc-bug.o
eci-eeprom: eci-eeprom.o pusb.o

pusb.o : pusb.c pusb-linux.c pusb.h
eci-load1.o: eci-load1.c modem.h pusb.h
eci-load2.o: eci-load2.c modem.h pusb.h
pppoeci.o: pppoeci.c modem.h pusb.h
check-hdlc.o: check-hdlc.c
check-hdlc-bug.o: check-hdlc-bug.c
eci-eeprom.o: eci-eeprom.c modem.h pusb.h

install: all check-char-major-108 check-tty-ldisc-14 check-tty-ldisc-13 \
  install-etc-ppp-peers-adsl
	install -g root -d $(binprefix)
	install -m 500 eci-load1 eci-load2 eci_firm_kit_wanadoo.bin \
	eci_wan3.bin pppoeci startmodem $(binprefix)
	@echo "don't forget to edit  $(ETCDIR)/ppp/peers/adsl and  $(ETCDIR)/ppp/chap-secrets files"

clean:
	$(RM) ${BIN} *.o

# for for the ppp_generic alias in /etc/modules.conf
check-char-major-108:
	@grep "^alias char-major-108" $(ETCDIR)/modules.conf > /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "adding ppp_generic alias ..." ; \
		echo "alias char-major-108 ppp_generic" >> $(ETCDIR)/modules.conf ; \
	fi

# check for ppp_synctty alias in /etc/modules.conf
check-tty-ldisc-14:
	@grep "^alias tty-ldisc-14" $(ETCDIR)/modules.conf > /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "adding ppp_synctty alias ..." ;\
		echo "alias tty-ldisc-14 ppp_synctty" >> $(ETCDIR)/modules.conf ; \
	fi

# check for the n_hdlc alias in /etc/modules.conf
check-tty-ldisc-13:
	@grep "^alias tty-ldisc-13" $(ETCDIR)/modules.conf > /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "adding n_hdlc alias ..." ;\
		echo "alias tty-ldisc-13 n_hdlc" >> $(ETCDIR)/modules.conf ; \
	fi

# install the /etc/ppp/peers/adsl file
install-etc-ppp-peers-adsl:
	@if [ -f $(ETCDIR)/ppp/peers/adsl ]; then \
		echo "modifying existing $(ETCDIR)/ppp/peers/adsl" ; \
		user=`grep "^user " $(ETCDIR)/ppp/peers/adsl` ; \
		persist=`grep "^persist" $(ETCDIR)/ppp/peers/adsl` ; \
		grep -v "^user " adsl > $(ETCDIR)/ppp/peers/adsl ; \
		echo $$user >> $(ETCDIR)/ppp/peers/adsl ; \
		echo $$persist >> $(ETCDIR)/ppp/peers/adsl ; \
	else \
		echo "creating $(ETCDIR)/ppp/peers/adsl" ; \
		cp adsl $(ETCDIR)/ppp/peers/adsl ; \
		chmod 700 $(ETCDIR)/ppp/peers/adsl ; \
		chown root:root $(ETCDIR)/ppp/peers/adsl ; \
	fi
uninstall:
	$(RM) $(binprefix)/eci-load1
	$(RM) $(binprefix)/eci-load2
	$(RM) $(binprefix)/eci_firm_kit_wanadoo.bin
	$(RM) $(binprefix)/eci_wan3.bin
	$(RM) $(binprefix)/pppoeci
	$(RM) $(binprefix)/startmodem 
	@echo "The driver has been successfully uninstalled"

