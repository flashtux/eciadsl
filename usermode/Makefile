CFLAGS += -Wall -ansi -pedantic
LDFLAGS += -lpthread

PREFIX = /usr/local
BIN_DIR = $(PREFIX)/bin
CONF_DIR = /etc/eciadsl
PPP_DIR = /etc/ppp

INSTALL_EXEC = install -m 755
INSTALL_DATA = install -m 644
INSTALL_DIR = install -d -m 755

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
	$(INSTALL_DIR) $(CONF_DIR)
	$(INSTALL_DATA) adsl $(CONF_DIR)/adsl-skel
	$(INSTALL_DATA) firmware00.bin synch01.bin modemeci.gif $(CONF_DIR)
	ln -fs $(CONF_DIR)/firmware00.bin $(CONF_DIR)/firmware.bin
	ln -fs $(CONF_DIR)/synch01.bin $(CONF_DIR)/synch.bin
	$(INSTALL_DIR) $(BIN_DIR)
	$(INSTALL_EXEC) startmodem eci-load1 eci-load2 pppoeci eci-doctor.sh \
		check-hdlc check-hdlc-bug eciconf.sh remove_dabusb makeconfig \
		eciconftxt.sh probe_device.sh $(BIN_DIR)
	@echo "don't forget to edit $(PPP_DIR)/peers/adsl and $(PPP_DIR)/chap-secrets files"

clean:
	$(RM) ${BIN} *.o

modules_conf:
# for for the ppp_generic, ppp_synctty and n_hdlc aliases in /etc/modules.conf
	@test -f /etc/modules.conf
	@FILE=/etc/modules.conf; \
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
	@mkdir -p $(PPP_DIR)/peers
	@if [ -f $(PPP_DIR)/peers/adsl ]; then \
		echo "modifying existing $(PPP_DIR)/peers/adsl.."; \
		USER=`grep -E "^user[ \t]+" $(PPP_DIR)/peers/adsl`; \
		PERSIST=`grep "^persist" $(PPP_DIR)/peers/adsl`; \
		grep -vE "^user[ \t]+" adsl > $(PPP_DIR)/peers/adsl; \
		echo -e "$$USER\n$$PERSIST" >> $(PPP_DIR)/peers/adsl; \
	else \
		echo "creating $(PPP_DIR)/peers/adsl.."; \
		$(INSTALL_DATA) adsl $(PPP_DIR)/peers/adsl; \
	fi

uninstall: uninstall_bin_files
	$(RM) $(BIN_DIR)/startmodem
	$(RM) $(BIN_DIR)/eci-load1
	$(RM) $(BIN_DIR)/eci-load2
	$(RM) $(BIN_DIR)/pppoeci
	$(RM) $(BIN_DIR)/check-hdlc
	$(RM) $(BIN_DIR)/check-hdlc-bug
	$(RM) $(BIN_DIR)/eciconf.sh
	$(RM) $(BIN_DIR)/eciconftxt.sh
	$(RM) $(BIN_DIR)/eci-doctor.sh
	$(RM) $(BIN_DIR)/probe_device.sh
	$(RM) $(BIN_DIR)/remove_dabusb
	$(RM) $(BIN_DIR)/makeconfig
	$(RM) $(CONF_DIR)/adsl-skel
	$(RM) $(CONF_DIR)/modemeci.gif
	$(RM) -r $(CONF_DIR)
	@echo "the driver has been uninstalled"

uninstall_bin_files:
	$(RM) $(CONF_DIR)/firmware.bin
	$(RM) $(CONF_DIR)/synch.bin
	$(RM) $(CONF_DIR)/firmware00.bin
	$(RM) $(CONF_DIR)/synch01.bin
	$(RM) $(CONF_DIR)/eci_firm_kit_wanadoo.bin
	$(RM) $(CONF_DIR)/eci_wan.bin
	$(RM) $(CONF_DIR)/eci_wan3.bin
	$(RM) $(CONF_DIR)/eci_wan3.dmt.bin

debian: all
	@dpkg-buildpackage -rfakeroot

rpm:
	@echo "not yet implemented"
#   tar zcvf /tmp/eciadsl-usermode-x.y.tar.gz eciadsl-usermode-x.y
#	rpm -ta /tmp/eciadsl-usermode-x.y.tar.gz
#   RPMS : /usr/src/RPM/RPMS/i686/eciadsl-usermode-0.5-2.i686.rpm
#	SRPMS: /usr/src/RPM/SRPMS/eciadsl-usermode-0.5-2.src.rpm
