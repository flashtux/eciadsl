CFLAGS += -Wall -ansi -pedantic
LDFLAGS += -lpthread

PREFIX ?= 
ETCDIR = $(PREFIX)/etc
BINDIR = $(PREFIX)/usr/local/bin
CONFDIR = $(ETCDIR)/eciadsl
PPPDIR = $(ETCDIR)/ppp

INSTALL_EXEC = install -c -p -m 755
INSTALL_DATA = install -c -p -m 644
INSTALL_DIR = install -c -p -d -m 755

# Final targets
BIN = eci-load1 eci-load2 pppoeci check-hdlc check-hdlc-bug

all: ${BIN}

eci-load1: eci-load1.o pusb.o
eci-load2: eci-load2.o pusb.o
pppoeci: pppoeci.o pusb.o
eci-eeprom: eci-eeprom.o pusb.o
check-hdlc: check-hdlc.o
check-hdlc-bug: check-hdlc-bug.o

pusb.o : pusb.c pusb-linux.c pusb.h
eci-load1.o: eci-load1.c modem.h pusb.h
eci-load2.o: eci-load2.c modem.h pusb.h
pppoeci.o: pppoeci.c modem.h pusb.h
eci-eeprom.o: eci-eeprom.c modem.h pusb.h
check-hdlc.o: check-hdlc.c
check-hdlc-bug.o: check-hdlc-bug.c

install: all modules_conf ppp_peer uninstall_bin_files
	$(INSTALL_DIR) $(CONFDIR)
	$(INSTALL_DATA) adsl $(CONFDIR)/adsl-skel
	$(INSTALL_DATA) firmware00.bin synch01.bin modemeci.gif $(CONFDIR)
	ln -fs $(CONFDIR)/firmware00.bin $(CONFDIR)/firmware.bin
	ln -fs $(CONFDIR)/synch01.bin $(CONFDIR)/synch.bin
	$(INSTALL_DIR) $(BINDIR)
	$(INSTALL_EXEC) startmodem eci-load1 eci-load2 pppoeci eci-doctor.sh \
		check-hdlc check-hdlc-bug eciconf.sh remove_dabusb makeconfig \
		eciconftxt.sh probe_device.sh $(BINDIR)
	@echo "don't forget to edit $(PPPDIR)/peers/adsl and $(PPPDIR)/chap-secrets files"

clean:
	$(RM) ${BIN} *.o

modules_conf:
# for for the ppp_generic, ppp_synctty and n_hdlc aliases in /etc/modules.conf
	@test -f $(ETCDIR)/modules.conf
	@FILE=$(ETCDIR)/modules.conf; \
	grep -E "^alias[ \t]+char-major-108[ \t]+" $$FILE > /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "adding ppp_generic alias.."; \
		echo "alias char-major-108 ppp_generic" >> $$FILE; \
	fi; \
	grep -E "^alias[ \t]+tty-ldisc-14[ \t]+" $$FILE > /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "adding ppp_synctty alias..";\
		echo "alias tty-ldisc-14 ppp_synctty" >> $$FILE; \
	fi; \
	grep -E "^alias[ \t]+tty-ldisc-13[ \t]+" $$FILE > /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "adding n_hdlc alias..";\
		echo "alias tty-ldisc-13 n_hdlc" >> $$FILE; \
	fi 

ppp_peer:
# install the /etc/ppp/peers/adsl file
	@mkdir -p $(PPPDIR)/peers
	@if [ -f $(PPPDIR)/peers/adsl ]; then \
		echo "modifying existing $(PPPDIR)/peers/adsl.."; \
		USER=`grep -E "^user[ \t]+" $(PPPDIR)/peers/adsl`; \
		PERSIST=`grep "^persist" $(PPPDIR)/peers/adsl`; \
		grep -vE "^user[ \t]+" adsl > $(PPPDIR)/peers/adsl; \
		echo -e "$$USER\n$$PERSIST" >> $(PPPDIR)/peers/adsl; \
	else \
		echo "creating $(PPPDIR)/peers/adsl.."; \
		$(INSTALL_DATA) adsl $(PPPDIR)/peers/adsl; \
	fi

uninstall: uninstall_bin_files
	$(RM) $(BINDIR)/startmodem
	$(RM) $(BINDIR)/eci-load1
	$(RM) $(BINDIR)/eci-load2
	$(RM) $(BINDIR)/pppoeci
	$(RM) $(BINDIR)/check-hdlc
	$(RM) $(BINDIR)/check-hdlc-bug
	$(RM) $(BINDIR)/eciconf.sh
	$(RM) $(BINDIR)/eciconftxt.sh
	$(RM) $(BINDIR)/eci-doctor.sh
	$(RM) $(BINDIR)/probe_device.sh
	$(RM) $(BINDIR)/remove_dabusb
	$(RM) $(BINDIR)/makeconfig
	$(RM) $(CONFDIR)/adsl-skel
	$(RM) $(CONFDIR)/modemeci.gif
	$(RM) -r $(CONFDIR)
	@echo "the driver has been uninstalled"

uninstall_bin_files:
	$(RM) $(CONFDIR)/firmware.bin
	$(RM) $(CONFDIR)/synch.bin
	$(RM) $(CONFDIR)/firmware00.bin
	$(RM) $(CONFDIR)/synch01.bin
	$(RM) $(CONFDIR)/eci_firm_kit_wanadoo.bin
	$(RM) $(CONFDIR)/eci_wan.bin
	$(RM) $(CONFDIR)/eci_wan3.bin
	$(RM) $(CONFDIR)/eci_wan3.dmt.bin

debian: all
	@dpkg-buildpackage -rfakeroot

rpm:
	@echo "not yet implemented"
#   tar zcvf /tmp/eciadsl-usermode-x.y.tar.gz eciadsl-usermode-x.y
#	rpm -ta /tmp/eciadsl-usermode-x.y.tar.gz
#   RPMS : /usr/src/RPM/RPMS/i686/eciadsl-usermode-0.5-2.i686.rpm
#	SRPMS: /usr/src/RPM/SRPMS/eciadsl-usermode-0.5-2.src.rpm
