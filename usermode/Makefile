include Makefile.config

PPPD_DIR = $(ETC_DIR)/ppp

# Final targets
TARGETS = eci-load1 eci-load2 pppoeci check-hdlc check-hdlc-bug
SCRIPTS = startmodem eci-doctor.sh eciconf.sh eciconftxt.sh probe_device.sh probe_synch.sh\
			remove_dabusb makeconfig eci_vendor_device.pl eci_data.pl eci_uc.pl

TMP_DIR = inst
TMP_FILE = Makefile.tmp

all: ${TARGETS} ${SCRIPTS}

eci-load1: eci-load1.o pusb.o util.o
eci-load2: eci-load2.o pusb.o util.o
pppoeci: pppoeci.o pusb.o gs7070.o util.o
eci-eeprom: eci-eeprom.o pusb.o
check-hdlc: check-hdlc.o
check-hdlc-bug: check-hdlc-bug.o

pusb.o : pusb.c pusb-linux.c pusb.h
eci-load1.o: eci-load1.c modem.h pusb.h
eci-load2.o: eci-load2.c modem.h pusb.h
pppoeci.o: pppoeci.c modem.h pusb.h
gs7070.o: gs7070.c gs7070.h
util.o: util.c util.h modem.h
eci-eeprom.o: eci-eeprom.c modem.h pusb.h
check-hdlc.o: check-hdlc.c
check-hdlc-bug.o: check-hdlc-bug.c

install: install_no_system_fixes modules_conf

install-strip: install
	for FILE in $(TARGETS); \
	do \
		if [ -x $(BIN_DIR)/$$FILE ]; \
		then \
			$(STRIP) $(BIN_DIR)/$$FILE; \
		fi; \
	done

install_no_system_fixes: all install_dirs install_scripts uninstall_old_bin_files
	$(INSTALL_DATA) firmware00.bin synch01.bin modemeci.gif providers.db modems.db $(CONF_DIR)
	ln -fs $(CONF_DIR)/firmware00.bin $(CONF_DIR)/firmware.bin
	ln -fs $(CONF_DIR)/synch01.bin $(CONF_DIR)/synch.bin
	$(INSTALL_EXEC) $(TARGETS) $(BIN_DIR)

install_scripts:
# patch scripts with useful pathes, use sed filter for perl, tcl and shell scripts
# filtered files are in $(TMP_DIR) then installed to $(BIN_DIR)
	$(INSTALL_DIR) $(TMP_DIR)
	@bindir=$$(echo "$(BIN_DIR)" | sed "s/\//\\\\\//g"); \
	etcdir=$$(echo "$(ETC_DIR)" | sed "s/\//\\\\\//g"); \
	confdir=$$(echo "$(CONF_DIR)" | sed "s/\//\\\\\//g"); \
	pppddir=$$(echo "$(PPPD_DIR)" | sed "s/\//\\\\\//g"); \
	version=$$(echo "$(PRODUCT_VERSION)" | sed "s/\//\\\\\//g"); \
	echo "s/set BIN_DIR .\+/set BIN_DIR \"$$bindir\"/" > $(TMP_DIR)/tcl.sed; \
	echo "s/set ETC_DIR .\+/set ETC_DIR \"$$etcdir\"/" >> $(TMP_DIR)/tcl.sed; \
	echo "s/set CONF_DIR .\+/set CONF_DIR \"$$confdir\"/" >> $(TMP_DIR)/tcl.sed; \
	echo "s/set PPPD_DIR .\+/set PPPD_DIR \"$$pppddir\"/" >> $(TMP_DIR)/tcl.sed; \
	echo "s/set VERSION .\+/set VERSION \"$$version\"/" >> $(TMP_DIR)/tcl.sed; \
	echo "s/\$$BIN_DIR =.\+/\$$BIN_DIR = \"$$bindir\";/" > $(TMP_DIR)/pl.sed; \
	echo "s/\$$ETC_DIR =.\+/\$$ETC_DIR = \"$$etcdir\";/" >> $(TMP_DIR)/pl.sed; \
	echo "s/\$$CONF_DIR =.\+/\$$CONF_DIR = \"$$confdir\";/" >> $(TMP_DIR)/pl.sed; \
	echo "s/\$$PPPD_DIR =.\+/\$$PPPD_DIR = \"$$pppddir\";/" >> $(TMP_DIR)/pl.sed; \
	echo "s/\$$VERSION =.\+/\$$VERSION = \"$$version\";/" >> $(TMP_DIR)/pl.sed; \
	echo "s/BIN_DIR=.\+/BIN_DIR=\"$$bindir\"/" > $(TMP_DIR)/sh.sed; \
	echo "s/ETC_DIR=.\+/ETC_DIR=\"$$etcdir\"/" >> $(TMP_DIR)/sh.sed; \
	echo "s/CONF_DIR=.\+/CONF_DIR=\"$$confdir\"/" >> $(TMP_DIR)/sh.sed; \
	echo "s/PPPD_DIR=.\+/PPPD_DIR=\"$$pppddir\"/" >> $(TMP_DIR)/sh.sed; \
	echo "s/VERSION=.\+/VERSION=\"$$version\"/" >> $(TMP_DIR)/sh.sed;
	@for FILE in $(SCRIPTS); do \
		head -50 $$FILE | grep -E "^exec wish" > /dev/null; \
		if [ $$? -eq 0 ]; then \
			sed -f $(TMP_DIR)/tcl.sed $$FILE > $(TMP_DIR)/$$FILE; \
		else \
			head -1 $$FILE | grep -E "/bin/perl" > /dev/null; \
			if [ $$? -eq 0 ]; then \
				sed -f $(TMP_DIR)/pl.sed $$FILE > $(TMP_DIR)/$$FILE; \
			else \
				sed -f $(TMP_DIR)/sh.sed $$FILE > $(TMP_DIR)/$$FILE; \
			fi; \
		fi; \
		$(INSTALL_EXEC) $(TMP_DIR)/$$FILE $(BIN_DIR); \
	done

install_dirs:
	if [ -n "$(PREFIX)" -a "$(PREFIX)" != "/" ]; then $(INSTALL_DIR) $(PREFIX); fi
	if [ -n "$(ETC_PREFIX)" -a "$(ETC_PREFIX)" != "/" ]; then $(INSTALL_DIR) $(ETC_PREFIX); fi
	if [ -n "$(CONF_PREFIX)" -a "$(CONF_PREFIX)" != "/" ]; then $(INSTALL_DIR) $(CONF_PREFIX); fi
	$(INSTALL_DIR) $(BIN_DIR)
	$(INSTALL_DIR) $(CONF_DIR)
	$(INSTALL_DIR) $(PPPD_DIR)
	$(INSTALL_DIR) $(PPPD_DIR)/peers

clean:
	$(RM) ${TARGETS} *.o

distclean: clean
	$(RM) Makefile.config config.h
	@for FILE in $(SCRIPTS); do \
		$(RM) $(TMP_DIR)/$$FILE; \
	done
	$(RM) $(TMP_DIR)/*.sed
	$(RM) -r $(TMP_DIR)
	@./configure --clean

modules_conf:
# check/set ppp_generic, ppp_synctty and n_hdlc aliases in /etc/modules.conf
# TODO: after this: touch $$FILE /lib/modules/*/modules.dep to prevent system
# messages when modprobe is used later
	@FILE=$(ETC_DIR)/modules.conf; \
	if [ -f $$FILE ]; then \
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
		fi; \
	fi

uninstall: uninstall_old_bin_files uninstall_dirs uninstall_bin uninstall_scripts
	$(RM) $(CONF_DIR)/modemeci.gif
	@echo "the driver has been uninstalled"

uninstall_bin:
	@for FILE in $(TARGETS); do \
		$(RM) $(BIN_DIR)/$$FILE; \
	done

uninstall_scripts:
	@for FILE in $(SCRIPTS); do \
		$(RM) $(BIN_DIR)/$$FILE; \
	done

uninstall_old_bin_files:
	$(RM) $(CONF_DIR)/firmware.bin
	$(RM) $(CONF_DIR)/synch.bin
	$(RM) $(CONF_DIR)/firmware00.bin
	$(RM) $(CONF_DIR)/synch01.bin
	$(RM) $(CONF_DIR)/eci_firm_kit_wanadoo.bin
	$(RM) $(CONF_DIR)/eci_wan.bin
	$(RM) $(CONF_DIR)/eci_wan3.bin
	$(RM) $(CONF_DIR)/eci_wan3.dmt.bin
	$(RM) $(CONF_DIR)/vidpid
	$(RM) $(CONF_DIR)/adsl-skel

uninstall_dirs:
	$(RM) -r $(CONF_DIR)

debian: all
	@dpkg-buildpackage -rfakeroot

rpm:
	@echo "not yet implemented"
#   tar zcvf /tmp/eciadsl-usermode-x.y.tar.gz eciadsl-usermode-x.y
#	rpm -ta /tmp/eciadsl-usermode-x.y.tar.gz
#   RPMS : /usr/src/RPM/RPMS/i686/eciadsl-usermode-0.5-2.i686.rpm
#	SRPMS: /usr/src/RPM/SRPMS/eciadsl-usermode-0.5-2.src.rpm

cfg:
	echo -e "#!/bin/sh\n# \\\\\nexec wish \"$(TMP_FILE)\"\nexit 0\n" > $(TMP_FILE)
	sh $(TMP_FILE) && $(BIN_DIR)/eciconf.sh || $(BIN_DIR)/eciconftxt.sh
	$(RM) $(TMP_FILE)
