include Makefile.config

PPPD_DIR = $(ETC_DIR)/ppp
ROOT ?= /
RPM_ARCH ?= i386,i686

# Final targets
TARGETS = eci-load1 eci-load2 pppoeci check-hdlc check-hdlc-bug
SCRIPTS = startmodem eci-doctor.sh eciconf.sh eciconftxt.sh probe_device.sh probe_synch.sh\
			remove_dabusb makeconfig eci_vendor_device.pl eci_data.pl eci_uc.pl
DOCS	= README INSTALL TROUBLESHOOTING README.fr INSTALL.fr TROUBLESHOOTING.fr BUGS TODO

TMP_DIR = /tmp/inst
TMP_FILE = Makefile.tmp

all: check ${TARGETS} ${SCRIPTS}

eci-load1: eci-load1.o pusb.o util.o
eci-load2: eci-load2.o pusb.o util.o semaphore.o
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
semaphore.o: semaphore.c semaphore.h
eci-eeprom.o: eci-eeprom.c modem.h pusb.h
check-hdlc.o: check-hdlc.c
check-hdlc-bug.o: check-hdlc-bug.c

check:
	@if [ "$(PRODUCT_VERSION)" != "$$(cat VERSION)" ]; then \
		echo "Version has changed since your last ./configure call"; \
		echo "You should re-run ./configure"; \
	fi

install: check install_no_system_fixes modules_conf

install-strip: install
	for FILE in $(TARGETS); \
	do \
		if [ -x $(ROOT)$(BIN_DIR)/$$FILE ]; \
		then \
			$(STRIP) $(ROOT)$(BIN_DIR)/$$FILE; \
		fi; \
	done

install_no_system_fixes: all install_dirs install_scripts install_docs uninstall_old_bin_files
	$(INSTALL_DATA) firmware00.bin synch01.bin modemeci.gif providers.db modems.db $(ROOT)$(CONF_DIR)
	$(INSTALL_EXEC) $(TARGETS) $(ROOT)$(BIN_DIR)

install_scripts:
# patch scripts with useful pathes, use sed filter for perl, tcl and shell scripts
# filtered files are in $(TMP_DIR) then installed to $(BIN_DIR)
	$(INSTALL_DIR) $(ROOT)$(TMP_DIR)
	@bindir=$$(echo "$(BIN_DIR)" | sed "s/\//\\\\\//g"); \
	etcdir=$$(echo "$(ETC_DIR)" | sed "s/\//\\\\\//g"); \
	confdir=$$(echo "$(CONF_DIR)" | sed "s/\//\\\\\//g"); \
	pppddir=$$(echo "$(PPPD_DIR)" | sed "s/\//\\\\\//g"); \
	version=$$(echo "$(PRODUCT_VERSION)" | sed "s/\//\\\\\//g"); \
	echo "s/set BIN_DIR .\+/set BIN_DIR \"$$bindir\"/" > $(ROOT)$(TMP_DIR)/tcl.sed; \
	echo "s/set ETC_DIR .\+/set ETC_DIR \"$$etcdir\"/" >> $(ROOT)$(TMP_DIR)/tcl.sed; \
	echo "s/set CONF_DIR .\+/set CONF_DIR \"$$confdir\"/" >> $(ROOT)$(TMP_DIR)/tcl.sed; \
	echo "s/set PPPD_DIR .\+/set PPPD_DIR \"$$pppddir\"/" >> $(ROOT)$(TMP_DIR)/tcl.sed; \
	echo "s/set VERSION .\+/set VERSION \"$$version\"/" >> $(ROOT)$(TMP_DIR)/tcl.sed; \
	echo "s/\$$BIN_DIR =.\+/\$$BIN_DIR = \"$$bindir\";/" > $(ROOT)$(TMP_DIR)/pl.sed; \
	echo "s/\$$ETC_DIR =.\+/\$$ETC_DIR = \"$$etcdir\";/" >> $(ROOT)$(TMP_DIR)/pl.sed; \
	echo "s/\$$CONF_DIR =.\+/\$$CONF_DIR = \"$$confdir\";/" >> $(ROOT)$(TMP_DIR)/pl.sed; \
	echo "s/\$$PPPD_DIR =.\+/\$$PPPD_DIR = \"$$pppddir\";/" >> $(ROOT)$(TMP_DIR)/pl.sed; \
	echo "s/\$$VERSION =.\+/\$$VERSION = \"$$version\";/" >> $(ROOT)$(TMP_DIR)/pl.sed; \
	echo "s/BIN_DIR=.\+/BIN_DIR=\"$$bindir\"/" > $(ROOT)$(TMP_DIR)/sh.sed; \
	echo "s/ETC_DIR=.\+/ETC_DIR=\"$$etcdir\"/" >> $(ROOT)$(TMP_DIR)/sh.sed; \
	echo "s/CONF_DIR=.\+/CONF_DIR=\"$$confdir\"/" >> $(ROOT)$(TMP_DIR)/sh.sed; \
	echo "s/PPPD_DIR=.\+/PPPD_DIR=\"$$pppddir\"/" >> $(ROOT)$(TMP_DIR)/sh.sed; \
	echo "s/VERSION=.\+/VERSION=\"$$version\"/" >> $(ROOT)$(TMP_DIR)/sh.sed;
	@for FILE in $(SCRIPTS); do \
		head -50 $$FILE | grep -E "^exec wish" > /dev/null; \
		if [ $$? -eq 0 ]; then \
			sed -f $(ROOT)$(TMP_DIR)/tcl.sed $$FILE > $(ROOT)$(TMP_DIR)/$$FILE; \
		else \
			head -1 $$FILE | grep -E "/bin/perl" > /dev/null; \
			if [ $$? -eq 0 ]; then \
				sed -f $(ROOT)$(TMP_DIR)/pl.sed $$FILE > $(ROOT)$(TMP_DIR)/$$FILE; \
			else \
				sed -f $(ROOT)$(TMP_DIR)/sh.sed $$FILE > $(ROOT)$(TMP_DIR)/$$FILE; \
			fi; \
		fi; \
		$(INSTALL_EXEC) $(ROOT)$(TMP_DIR)/$$FILE $(ROOT)$(BIN_DIR); \
	done
	$(MAKE) externdistclean

install_docs:
	@for FILE in $(DOCS); do \
		$(INSTALL_DATA) $$FILE $(ROOT)$(DOC_DIR); \
	done

install_dirs:
	if [ -n "$(ROOT)$(PREFIX)" -a "$(ROOT)$(PREFIX)" != "/" ]; then $(INSTALL_DIR) $(ROOT)$(PREFIX); fi
	if [ -n "$(ROOT)$(ETC_PREFIX)" -a "$(ROOT)$(ETC_PREFIX)" != "/" ]; then $(INSTALL_DIR) $(ROOT)$(ETC_PREFIX); fi
	if [ -n "$(ROOT)$(CONF_PREFIX)" -a "$(ROOT)$(CONF_PREFIX)" != "/" ]; then $(INSTALL_DIR) $(ROOT)$(CONF_PREFIX); fi
	if [ -n "$(ROOT)$(DOC_PREFIX)" -a "$(ROOT)$(DOC_PREFIX)" != "/" ]; then $(INSTALL_DIR) $(ROOT)$(DOC_PREFIX); fi
	$(INSTALL_DIR) $(ROOT)$(BIN_DIR)
	$(INSTALL_DIR) $(ROOT)$(CONF_DIR)
	$(INSTALL_DIR) $(ROOT)$(DOC_DIR)
	$(INSTALL_DIR) $(ROOT)$(PPPD_DIR)
	$(INSTALL_DIR) $(ROOT)$(PPPD_DIR)/peers

clean:
	$(RM) ${TARGETS} *.o

distclean: clean externdistclean
	$(RM) Makefile.config config.h
	@for FILE in $(SCRIPTS); do \
		$(RM) $(ROOT)$(TMP_DIR)/$$FILE; \
	done
	@./configure --clean

externdistclean:
	$(RM) $(ROOT)$(TMP_DIR)/*.sed
	$(RM) -r $(ROOT)$(TMP_DIR)

modules_conf:
# check/set ppp_generic, ppp_synctty and n_hdlc aliases in /etc/modules.conf
# TODO: after this: touch $$FILE /lib/modules/*/modules.dep to prevent system
# messages when modprobe is used later
	@FILE=$(ROOT)$(ETC_DIR)/modules.conf; \
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

uninstall: uninstall_old_bin_files uninstall_dirs uninstall_bin uninstall_scripts uninstall_docs
	$(RM) $(ROOT)$(CONF_DIR)/modemeci.gif
	@echo "the driver has been uninstalled"

uninstall_bin:
	@for FILE in $(TARGETS); do \
		$(RM) $(ROOT)$(BIN_DIR)/$$FILE; \
	done

uninstall_scripts:
	@for FILE in $(SCRIPTS); do \
		$(RM) $(ROOT)$(BIN_DIR)/$$FILE; \
	done

uninstall_docs:
	@for FILE in $(DOCS); do \
		$(RM) $(ROOT)$(DOC_DIR)/$$FILE; \
	done

uninstall_old_bin_files:
	$(RM) $(ROOT)$(CONF_DIR)/firmware.bin
	$(RM) $(ROOT)$(CONF_DIR)/synch.bin
	$(RM) $(ROOT)$(CONF_DIR)/firmware00.bin
	$(RM) $(ROOT)$(CONF_DIR)/synch01.bin
	$(RM) $(ROOT)$(CONF_DIR)/eci_firm_kit_wanadoo.bin
	$(RM) $(ROOT)$(CONF_DIR)/eci_wan.bin
	$(RM) $(ROOT)$(CONF_DIR)/eci_wan3.bin
	$(RM) $(ROOT)$(CONF_DIR)/eci_wan3.dmt.bin
	$(RM) $(ROOT)$(CONF_DIR)/vidpid
	$(RM) $(ROOT)$(CONF_DIR)/adsl-skel

uninstall_dirs:
	$(RM) -r $(ROOT)$(CONF_DIR)
	$(RM) -r $(ROOT)$(DOC_DIR)

debian: all
	@dpkg-buildpackage -rfakeroot

rpm: rpmdistclean
	@test -n "$(CVS_TAG)" || { \
		echo -e "usage:\nmake -s CVS_TAG=\"release-0-6\" [RPM_ARCH=i386,i686] [PRODUCT_VERSION=0.6] rpm" >&2; \
		exit 1; }
	@echo "building .(s)rpm for eciadsl-usermode-$(PRODUCT_VERSION) (cvs tag $(CVS_TAG), target arch=$(RPM_ARCH))"
	-@mkdir /tmp/usermode
	cd /tmp && cvs -q -d $(ECIADSLCVS) co -r "$(CVS_TAG)" usermode
	mv /tmp/usermode /tmp/eciadsl-usermode-$(PRODUCT_VERSION)
	cd /tmp && tar -czf eciadsl-usermode-$(PRODUCT_VERSION).tar.gz eciadsl-usermode-$(PRODUCT_VERSION)
	rpm -ta --target=$(RPM_ARCH) /tmp/eciadsl-usermode-$(PRODUCT_VERSION).tar.gz
	@echo "RPM should be in /usr/src/RPM/RPMS or /usr/src/packages/RPMS (SuSE)"
	@echo "SRPMS should be /usr/src/RPM/SRPMS or /usr/src/packages/SRPMS (SuSE)"
	$(MAKE) rpmdistclean

rpmdistclean:
	-rm -Rf /tmp/eciadsl-usermode-$(PRODUCT_VERSION) /tmp/usermode
	-rm -f /tmp/eciadsl-usermode-$(PRODUCT_VERSION).tar.gz

cfg: check
	echo -e "#!/bin/sh\n# \\\\\nexec wish \"$(TMP_FILE)\"\nexit 0\n" > $(TMP_FILE)
	sh $(TMP_FILE) && $(ROOT)$(BIN_DIR)/eciconf.sh || $(ROOT)$(BIN_DIR)/eciconftxt.sh
	$(RM) $(TMP_FILE)
