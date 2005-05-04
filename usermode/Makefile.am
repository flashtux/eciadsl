SUBDIRS = eci-common

bin_PROGRAMS = eciadsl-firmware \
	eciadsl-synch eciadsl-pppoeci \
	eciadsl-check-hdlc eciadsl-check-hdlc-bug

dist_bin_SCRIPTS = eciadsl-start eciadsl-stop eciadsl-doctor \
	eciadsl-config-tk eciadsl-config-text \
	eciadsl-probe-device eciadsl-probe-synch \
	eciadsl-remove-dabusb eciadsl-makeconfig \
	eciadsl-vendor-device.pl eciadsl-data.pl eciadsl-uc.pl

eciadsl_firmware_SOURCES = eciadsl-firmware.c pusb.c pusb.h util.c util.h \
	gsinterface.c gsinterface.h

eciadsl_synch_SOURCES = eciadsl-synch.c semaphore.c semaphore.h pusb.c util.c \
	gsinterface.c

eciadsl_pppoeci_SOURCES = eciadsl-pppoeci.c pusb.c util.c \
	gsinterface.c
	
	
eciadsl_pppoeci_LDADD = eci-common/eoc.o

eciadsl_check_hdlc_SOURCES = eciadsl-check-hdlc.c

eciadsl_check_hdlc_bug_SOURCES = eciadsl-check-hdlc-bug.c

dist_sysconf_DATA = 

debian_DIST = debian/changelog debian/control debian/copyright debian/rules \
	debian/docs

rpm_DIST = eciadsl-usermode.spec eciadsl-usermode.spec.in

EXTRA_DIST = pusb-linux.c pusb-linux.h gs7470.c gs7470.h gs7070.c gs7070.h \
	eciadsl-start.in eciadsl-stop eciadsl-doctor.in \
	eciadsl-config-tk.in eciadsl-config-text.in \
	eciadsl-probe-device eciadsl-probe-synch \
	eciadsl-remove-dabusb.in eciadsl-makeconfig.in \
	eciadsl-vendor-device.pl eciadsl-data.pl eciadsl-uc.pl \
	$(etc_DIST) $(doc_DIST) $(debian_DIST) $(rpm_DIST)

CONF_DIR=$(sysconfdir)/eciadsl
DOC_DIR=$(datadir)/doc/eciadsl

AM_CPPFLAGS = -DETC_DIR=\"$(sysconfdir)\" -DCONF_DIR=\"$(CONF_DIR)\" \
	-DDOC_DIR=\"$(DOC_DIR)\"

PACKAGE_VERSION=@PACKAGE_VERSION@

# See info autoconf "Installation Directory Variables"
edit = sed \
	-e 's,\@bindir\@,$(bindir),g' \
	-e 's,\@sysconfdir\@,$(sysconfdir),g' \
	-e 's,\@PACKAGE_VERSION\@,$(PACKAGE_VERSION),g'

eciadsl-start: $(srcdir)/eciadsl-start.in Makefile
	rm -f $@ $@.tmp
	$(edit) < $< > $@.tmp
	mv $@.tmp $@

eciadsl-config-tk: $(srcdir)/eciadsl-config-tk.in Makefile
	rm -f $@ $@.tmp
	$(edit) < $< > $@.tmp
	mv $@.tmp $@

eciadsl-config-text: $(srcdir)/eciadsl-config-text.in Makefile
	rm -f $@ $@.tmp
	$(edit) < $< > $@.tmp
	mv $@.tmp $@

eciadsl-makeconfig: $(srcdir)/eciadsl-makeconfig.in Makefile
	rm -f $@ $@.tmp
	$(edit) < $< > $@.tmp
	mv $@.tmp $@

eciadsl-remove-dabusb: $(srcdir)/eciadsl-remove-dabusb.in Makefile
	rm -f $@ $@.tmp
	$(edit) < $< > $@.tmp
	mv $@.tmp $@

eciadsl-doctor: $(srcdir)/eciadsl-doctor.in Makefile
	rm -f $@ $@.tmp
	$(edit) < $< > $@.tmp
	mv $@.tmp $@

CLEANFILES = eciadsl-start eciadsl-config-tk eciadsl-config-text \
	eciadsl-makeconfig eciadsl-remove-dabusb eciadsl-doctor

# for files in /etc/eciadsl

etc_DIST = firmware00.bin modemeci.gif modems.db providers.db synch01.bin

install-data-local::
	@echo "DESTDIR=$(DESTDIR) CONF_DIR=$(CONF_DIR) DOC_DIR=$(DOC_DIR)"
	mkdir -p "$(DESTDIR)$(CONF_DIR)"
	for FILE in $(etc_DIST); do \
		$(INSTALL_DATA)	"$(srcdir)/$${FILE}" "$(DESTDIR)$(CONF_DIR)"; \
	done

uninstall-local::
	for FILE in $(etc_DIST); do \
		$(RM) "$(DESTDIR)$(CONF_DIR)/$${FILE}"; \
	done
	-rmdir "$(DESTDIR)$(CONF_DIR)"

# for files in /usr/share/doc/eciadsl

doc_DIST = README README.es README.fr README.it README.pt README_GS7470.txt \
	INSTALL INSTALL.es INSTALL.fr INSTALL.it INSTALL.pt \
	TROUBLESHOOTING TROUBLESHOOTING.es TROUBLESHOOTING.fr \
	TROUBLESHOOTING.it TROUBLESHOOTING.pt BUGS TODO rc.adsl

# We do not install documentation since:
# - we don't know the proper directory to install (/usr/share/doc, /usr/doc)
# - it will conflict with documentation installed by rpm or debian packages

#install-data-local::
#	mkdir -p "$(DESTDIR)$(DOC_DIR)"
#	for FILE in $(doc_DIST); do \
#		$(INSTALL_DATA) "$(srcdir)/$${FILE}" "$(DESTDIR)$(DOC_DIR)"; \
#	done

#uninstall-local::
#	for FILE in $(doc_DIST); do \
#		$(RM) "$(DESTDIR)$(DOC_DIR)/$${FILE}"; \
#	done
#	rmdir "$(DESTDIR)$(DOC_DIR)"
