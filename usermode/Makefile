# 23/12/2001 Benoit PAPILLAULT
# Licence GPL
#   make the file /etc/ppp/peers/adsl writable for user root (since most of
#   the time, he must change the "user").

# TODO: If the /etc/ppp/peers/adsl already exists, ask the user to overwrite it
# with the new one (but with keeping the same user and the persist option)

# install: should check for "tty-ldisc-13" in /etc/modules.conf. DONE.

# 15/02/2002 suggestion from Dumas Patrice <dumas@centre-cired.fr>
#   added ETCDIR, prefix and binprefix

CFLAGS += -Wall -g
LDFLAGS += -g -Wall -lpthread

# you can define ROOT by "make ROOT=/tmp" else it defaults to /
ROOT ?= /

prefix=$(ROOT)/usr/local
binprefix=$(prefix)/bin
ETCDIR=$(ROOT)/etc

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
# copy executable files to /usr/local/bin
	install -d $(binprefix)
	install startmodem eci-load1 eci-load2 pppoeci eci-doctor.sh \
		check-hdlc check-hdlc-bug eciconf.sh remove_dabusb makeconfig \
		$(binprefix)
# copy configuration files to /etc/eciadsl
	install -d $(ETCDIR)/eciadsl
	install adsl $(ETCDIR)/eciadsl/adsl-skel
	install eci_firm_kit_wanadoo.bin eci_wan3.bin eci_wan3.dmt.bin \
		modemeci.gif $(ETCDIR)/eciadsl
	ln -s /etc/eciadsl/eci_wan3.dmt.bin /etc/eciadsl/eci_wan.bin
    @echo "don't forget to edit  $(ETCDIR)/ppp/peers/adsl and  $(ETCDIR)/ppp/chap-secrets files"

clean:
	$(RM) ${BIN} *.o

# for for the ppp_generic alias in /etc/modules.conf
check-char-major-108:
	@mkdir -p $(ETCDIR)
	@grep "^alias char-major-108" $(ETCDIR)/modules.conf > /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "adding ppp_generic alias ..." ; \
		echo "alias char-major-108 ppp_generic" >> $(ETCDIR)/modules.conf ; \
	fi

# check for ppp_synctty alias in /etc/modules.conf
check-tty-ldisc-14:
	@mkdir -p $(ETCDIR)
	@grep "^alias tty-ldisc-14" $(ETCDIR)/modules.conf > /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "adding ppp_synctty alias ..." ;\
		echo "alias tty-ldisc-14 ppp_synctty" >> $(ETCDIR)/modules.conf ; \
	fi

# check for the n_hdlc alias in /etc/modules.conf
check-tty-ldisc-13:
	@mkdir -p $(ETCDIR)
	@grep "^alias tty-ldisc-13" $(ETCDIR)/modules.conf > /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "adding n_hdlc alias ..." ;\
		echo "alias tty-ldisc-13 n_hdlc" >> $(ETCDIR)/modules.conf ; \
	fi

# install the /etc/ppp/peers/adsl file
install-etc-ppp-peers-adsl:
	@mkdir -p $(ETCDIR)/ppp/peers
	@if [ -f $(ETCDIR)/ppp/peers/adsl ]; then \
		echo "modifying existing $(ETCDIR)/ppp/peers/adsl" ; \
		user=`grep "^user " $(ETCDIR)/ppp/peers/adsl` ; \
		persist=`grep "^persist" $(ETCDIR)/ppp/peers/adsl` ; \
		grep -v "^user " adsl > $(ETCDIR)/ppp/peers/adsl ; \
		echo $$user >> $(ETCDIR)/ppp/peers/adsl ; \
		echo $$persist >> $(ETCDIR)/ppp/peers/adsl ; \
	else \
		echo "creating $(ETCDIR)/ppp/peers/adsl" ; \
		install adsl $(ETCDIR)/ppp/peers/adsl ; \
	fi

uninstall:
	$(RM) $(binprefix)/startmodem
	$(RM) $(binprefix)/eci-load1
	$(RM) $(binprefix)/eci-load2
	$(RM) $(binprefix)/pppoeci
	$(RM) $(binprefix)/eci-doctor.sh
	$(RM) $(binprefix)/check-hdlc
	$(RM) $(binprefix)/check-hdlc-bug
	$(RM) $(binprefix)/eciconf.sh
	$(RM) $(binprefix)/remove_dabusb
	$(RM) $(binprefix)/makeconfig
	$(RM) $(ETCDIR)/eciadsl/adsl-skel
	$(RM) $(ETCDIR)/eciadsl/eci_firm_kit_wanadoo.bin
    $(RM) $(ETCDIR)/eciadsl/eci_wan.bin
	$(RM) $(ETCDIR)/eciadsl/eci_wan3.bin
	$(RM) $(ETCDIR)/eciadsl/eci_wan3.dmt.bin
	$(RM) $(ETCDIR)/eciadsl/modemeci.gif
	@echo "The driver has been successfully uninstalled"

debian: all
	dpkg-buildpackage -rfakeroot

#rpm:
#   tar zcvf /tmp/eciadsl-usermode-x.y.tar.gz eciadsl-usermode-x.y
#	rpm -ta /tmp/eciadsl-usermode-x.y.tar.gz
#   RPMS : /usr/src/RPM/RPMS/i686/eciadsl-usermode-0.5-2.i686.rpm
#	SRPMS: /usr/src/RPM/SRPMS/eciadsl-usermode-0.5-2.src.rpm
