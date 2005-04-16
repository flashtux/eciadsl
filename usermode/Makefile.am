bin_PROGRAMS = eciadsl-firmware \
	eciadsl-synch eciadsl-pppoeci \
	eciadsl-check-hdlc eciadsl-check-hdlc-bug

bin_SCRIPTS = eciadsl-start eciadsl-stop eciadsl-doctor \
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

eciadsl_check_hdlc_SOURCES = eciadsl-check-hdlc.c

eciadsl_check_hdlc_bug_SOURCES = eciadsl-check-hdlc-bug.c

debian_DIST = debian/changelog debian/control debian/copyright debian/rules \
	debian/docs

rpm_DIST = eciadsl-usermode.spec

EXTRA_DIST = pusb-linux.c pusb-linux.h gs7470.c gs7470.h gs7070.c gs7070.h \
	eciadsl-stop eciadsl-probe-device eciadsl-probe-synch \
	eciadsl-vendor-device.pl eciadsl-data.pl eciadsl-uc.pl $(debian_DIST) \
	$(rpm_DIST)

AM_CPPFLAGS = -DETC_DIR=\"@sysconfdir@\" -DCONF_DIR=\"@sysconfdir@/eciadsl\" \
	-DDOC_DIR=\"@prefix@/doc/eciadsl@\"

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

