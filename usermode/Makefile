include Makefile.config

# for debugging with gdb, uncomment the following line
#CFLAGS += -g

PPPD_DIR = $(ETC_DIR)/ppp
RPM_ARCH ?= i386,i686

# Final targets
TARGETS = eciadsl-firmware eciadsl-synch eciadsl-pppoeci \
	eciadsl-check-hdlc eciadsl-check-hdlc-bug
SCRIPTS = eciadsl-start eciadsl-stop eciadsl-doctor \
	eciadsl-config-tk eciadsl-config-text\
	eciadsl-probe-device eciadsl-probe-synch\
	eciadsl-remove-dabusb eciadsl-makeconfig\
	eciadsl-vendor-device.pl eciadsl-data.pl eciadsl-uc.pl
DOCS = README README.fr INSTALL INSTALL.fr \
	TROUBLESHOOTING TROUBLESHOOTING.fr BUGS TODO

TMP_DIR = $(ROOT)/tmp/inst
TMP_FILE = Makefile.tmp

all: check $(TARGETS) $(SCRIPTS)
#	cd hotplug && $(MAKE)

eciadsl-firmware: eciadsl-firmware.o pusb.o util.o
eciadsl-synch: eciadsl-synch.o pusb.o util.o semaphore.o
eciadsl-pppoeci: eciadsl-pppoeci.o pusb.o gs7070.o util.o
pppatm: pppatm.o 
	$(CC) $(LDFLAGS) -latm -o pppatm pppatm.o
eciadsl-eeprom: eciadsl-eeprom.o pusb.o
eciadsl-check-hdlc: eciadsl-check-hdlc.o
eciadsl-check-hdlc-bug: eciadsl-check-hdlc-bug.o
pusb.o: pusb.c pusb-linux.c pusb.h
eciadsl-firmware.o: eciadsl-firmware.c pusb.h modem.h config.h util.h
eciadsl-synch.o: eciadsl-synch.c pusb.h modem.h config.h util.h semaphore.h
eciadsl-pppoeci.o: eciadsl-pppoeci.c util.h pusb.h gs7070.h modem.h config.h
gs7070.o: gs7070.c gs7070.h
util.o: util.c modem.h config.h util.h
semaphore.o: semaphore.c semaphore.h
	$(CC) $(CFLAGS)  -D _XOPEN_SOURCE -c -o semaphore.o semaphore.c
eciadsl-eeprom.o: eciadsl-eeprom.c pusb.h modem.h config.h
eciadsl-check-hdlc.o: eciadsl-check-hdlc.c
	$(CC) $(CFLAGS)  -D _XOPEN_SOURCE -c -o eciadsl-check-hdlc.o eciadsl-check-hdlc.c
eciadsl-check-hdlc-bug.o: eciadsl-check-hdlc-bug.c
	$(CC) $(CFLAGS)  -D _XOPEN_SOURCE -c -o eciadsl-check-hdlc-bug.o eciadsl-check-hdlc-bug.c

check:
	@if [ "$(PRODUCT_VERSION)" != "$$(cat VERSION)" ]; then \
		echo "Version has changed since your last ./configure call"; \
		echo "You should re-run ./configure"; \
	fi

install: check install_no_system_fixes modules_conf
#	cd hotplug && $(MAKE) install
	@echo "EciAdsl driver installed!"
	@echo "Now issue 'make cfg' command to configure driver."

install-strip: install
	for FILE in $(TARGETS); \
	do \
		if [ -x "$(ROOT)/$(BIN_DIR)/$${FILE}" ]; \
		then \
			$(STRIP) "$(ROOT)/$(BIN_DIR)/$${FILE}"; \
		fi; \
	done

install_no_system_fixes: all install_dirs install_scripts install_docs install_links uninstall_old_bin_files
	$(INSTALL_DATA) firmware00.bin synch01.bin modemeci.gif providers.db modems.db "$(ROOT)/$(CONF_DIR)"
	$(INSTALL_EXEC) $(TARGETS) "$(ROOT)/$(BIN_DIR)"

install_scripts:
# patch scripts with useful pathes, use sed filter for perl, tcl and
# shell scripts filtered files are in $(TMP_DIR) then installed to
# $(BIN_DIR)
	$(INSTALL_DIR) "$(TMP_DIR)"
	(echo "s|set BIN_DIR .*|set BIN_DIR \"$(BIN_DIR)\"|" ; \
	 echo "s|set ETC_DIR .*|set ETC_DIR \"$(ETC_DIR)\"|" ; \
	 echo "s|set CONF_DIR .*|set CONF_DIR \"$(CONF_DIR)\"|" ; \
	 echo "s|set PPPD_DIR .*|set PPPD_DIR \"$(PPPD_DIR)\"|" ; \
	 echo "s|set VERSION .*|set VERSION \"$(PRODUCT_VERSION)\"|" ;\
	) > "$(TMP_DIR)/tcl.sed";
	(echo "s|\$$BIN_DIR = .*|\$$BIN_DIR = \"$(BIN_DIR)\";|" ; \
	 echo "s|\$$ETC_DIR = .*|\$$ETC_DIR = \"$(ETC_DIR)\";|" ; \
	 echo "s|\$$CONF_DIR = .*|\$$CONF_DIR = \"$(CONF_DIR)\";|" ; \
	 echo "s|\$$PPPD_DIR =.*|\$$PPPD_DIR = \"$(PPPD_DIR)\";|" ; \
	 echo "s|\$$VERSION = .*|\$$VERSION = \"$(PRODUCT_VERSION)\";|" ; \
	) > "$(TMP_DIR)/pl.sed";
	(echo "s|BIN_DIR=.*|BIN_DIR=\"$(BIN_DIR)\"|" ;\
	 echo "s|ETC_DIR=.*|ETC_DIR=\"$(ETC_DIR)\"|" ; \
	 echo "s|CONF_DIR=.*|CONF_DIR=\"$(CONF_DIR)\"|" ; \
	 echo "s|PPPD_DIR=.*|PPPD_DIR=\"$(PPPD_DIR)\"|" ; \
	 echo "s|VERSION=.*|VERSION=\"$(PRODUCT_VERSION)\"|" ;\
	) > "$(TMP_DIR)/sh.sed";
	for FILE in $(SCRIPTS); do \
		head -n 50 $$FILE | grep -q -E "^exec wish"; \
		if [ $$? -eq 0 ]; then \
			sed -f "$(TMP_DIR)/tcl.sed" "$${FILE}" > "$(TMP_DIR)/$${FILE}"; \
		else \
			head -n 1 $$FILE | grep -q -E "/bin/perl"; \
			if [ $$? -eq 0 ]; then \
				sed -f "$(TMP_DIR)/pl.sed" "$${FILE}" > "$(TMP_DIR)/$${FILE}";\
			else \
				sed -f "$(TMP_DIR)/sh.sed" "$${FILE}" > "$(TMP_DIR)/$${FILE}";\
			fi; \
		fi; \
		$(INSTALL_EXEC) "$(TMP_DIR)/$${FILE}" "$(ROOT)/$(BIN_DIR)"; \
	done
	$(MAKE) externdistclean

install_docs:
	@for FILE in $(DOCS); do \
		$(INSTALL_DATA) "$${FILE}" "$(ROOT)/$(DOC_DIR)"; \
	done

install_links:
	ln -sf eciadsl-start    "$(ROOT)/$(BIN_DIR)/startmodem"
	ln -sf eciadsl-firmware "$(ROOT)/$(BIN_DIR)/eci-load1"
	ln -sf eciadsl-synch    "$(ROOT)/$(BIN_DIR)/eci-load2"
	ln -sf eciadsl-pppoeci  "$(ROOT)/$(BIN_DIR)/pppoeci"

install_dirs:
	$(INSTALL_DIR) "$(ROOT)/$(BIN_DIR)"
	$(INSTALL_DIR) "$(ROOT)/$(CONF_DIR)"
	$(INSTALL_DIR) "$(ROOT)/$(DOC_DIR)"
	$(INSTALL_DIR) "$(ROOT)/$(PPPD_DIR)"
	$(INSTALL_DIR) "$(ROOT)/$(PPPD_DIR)/peers"

clean:
	$(RM) $(TARGETS) *.o core

distclean: clean externdistclean
	$(RM) Makefile.config config.h
	@for FILE in $(SCRIPTS); do \
		$(RM) "$(TMP_DIR)/$${FILE}"; \
	done
	@./configure --clean

externdistclean:
	$(RM) "$(TMP_DIR)"/*.sed
	$(RM) -r "$(TMP_DIR)"

modules_conf:
# check/set ppp_generic, ppp_synctty and n_hdlc aliases in /etc/modules.conf
# TODO: after this: touch $$FILE /lib/modules/*/modules.dep to prevent system
# messages when modprobe is used later
	@FILE="$(ROOT)/$(ETC_DIR)/modules.conf"; \
	if [ -f "$${FILE}" ]; then \
		if ! grep -q -E "^alias[ \t]+char-major-108[ \t]+" $$FILE; then \
			echo "adding ppp_generic alias.."; \
			echo "alias char-major-108 ppp_generic" >> $$FILE; \
		fi; \
		if ! grep -q -E "^alias[ \t]+tty-ldisc-14[ \t]+" $$FILE; then \
			echo "adding ppp_synctty alias..";\
			echo "alias tty-ldisc-14 ppp_synctty" >> $$FILE; \
		fi; \
		if ! grep -q -E "^alias[ \t]+tty-ldisc-13[ \t]+" $$FILE; then \
			echo "adding n_hdlc alias..";\
			echo "alias tty-ldisc-13 n_hdlc" >> $$FILE; \
		fi; \
	fi

uninstall: uninstall_old_bin_files uninstall_dirs uninstall_bin uninstall_links uninstall_scripts uninstall_docs
#	cd hotplug && $(MAKE) uninstall
	$(RM) "$(ROOT)/$(CONF_DIR)/modemeci.gif"
	@echo "EciAdsl driver has been uninstalled."

uninstall_bin:
	@for FILE in $(TARGETS); do \
		$(RM) "$(ROOT)/$(BIN_DIR)/$${FILE}"; \
	done

uninstall_links:
	$(RM) "$(ROOT)/$(BIN_DIR)/startmodem"
	$(RM) "$(ROOT)/$(BIN_DIR)/eci-load1"
	$(RM) "$(ROOT)/$(BIN_DIR)/eci-load2"
	$(RM) "$(ROOT)/$(BIN_DIR)/pppoeci"

uninstall_scripts:
	@for FILE in $(SCRIPTS); do \
		$(RM) "$(ROOT)/$(BIN_DIR)/$${FILE}"; \
	done

uninstall_docs:
	@for FILE in $(DOCS); do \
		$(RM) "$(ROOT)/$(DOC_DIR)/$${FILE}"; \
	done

uninstall_old_bin_files:
	$(RM) "$(ROOT)/$(CONF_DIR)/firmware.bin"
	$(RM) "$(ROOT)/$(CONF_DIR)/synch.bin"
	$(RM) "$(ROOT)/$(CONF_DIR)/firmware00.bin"
	$(RM) "$(ROOT)/$(CONF_DIR)/synch01.bin"
	$(RM) "$(ROOT)/$(CONF_DIR)/eci_firm_kit_wanadoo.bin"
	$(RM) "$(ROOT)/$(CONF_DIR)/eci_wan.bin"
	$(RM) "$(ROOT)/$(CONF_DIR)/eci_wan3.bin"
	$(RM) "$(ROOT)/$(CONF_DIR)/eci_wan3.dmt.bin"
	$(RM) "$(ROOT)/$(CONF_DIR)/vidpid"
	$(RM) "$(ROOT)/$(CONF_DIR)/adsl-skel"

uninstall_dirs:
	$(RM) -r "$(ROOT)/$(CONF_DIR)"
	$(RM) -r "$(ROOT)/$(DOC_DIR)"

debian: all
	@dpkg-buildpackage -rfakeroot

rpm: rpmdistclean
	@test -n "$(CVS_TAG)" || { \
		echo -e "usage:\nmake -s CVS_TAG=\"release-0-6\" [ECIADSLCVS=\":ext:username@cvs.eciadsl.sourceforge.net:/cvsroot/eciadsl\"] [RPM_ARCH=i386,i686] [PRODUCT_VERSION=0.6] rpm" >&2; \
		exit 1; }
	@test -n "$(ECIADSLCVS)" || { \
		echo -e "you must define an environment variable ECIADSLCVS that contains the full cvs login," >&2; \
		echo -e "for instance: export ECIADSLCVS=\":ext:username@cvs.eciadsl.sourceforge.net:/cvsroot/eciadsl\"" >&2; \
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
	-@$(RM) -R /tmp/eciadsl-usermode-$(PRODUCT_VERSION) /tmp/usermode
	-@$(RM) /tmp/eciadsl-usermode-$(PRODUCT_VERSION).tar.gz

tgz:
	tar -C /tmp/eciadsl-usermode-$(PRODUCT_VERSION)-tgz -czf /tmp/eciadsl-usermode-$(PRODUCT_VERSION).tgz usr/ etc/

tgzdistclean:
	-@$(RM) -R /tmp/eciadsl-usermode-$(PRODUCT_VERSION)-tgz
	-@$(RM) /tmp/eciadsl-usermode-$(PRODUCT_VERSION).tgz

cfg: check
	echo -e "#!/bin/sh\n# \\\\\nexec wish \"$(TMP_FILE)\"\nexit 0\n" > "$(TMP_FILE)"
	sh "$(TMP_FILE)" && "$(ROOT)/$(BIN_DIR)/eciadsl-config-tk" || "$(ROOT)/$(BIN_DIR)/eciadsl-config-text"
	$(RM) "$(TMP_FILE)"
