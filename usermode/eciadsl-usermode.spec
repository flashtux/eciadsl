# to build : rpm -ba eciadsl-usermode.spec
# or       : rpm -ta eciadsl-usermode-0.10.tar.gz
# the tar.gz should contain a directory eciadsl-usermode-0.10

Summary:	A beta-quality usermode driver for the ECI ADSL USB modem
Name:     eciadsl-usermode
Version:	0.10
Release:	1
URL:      http://eciadsl.flashtux.org/
License:	GPL
Group:		Networking/Other
Packager:	David Faure <david@mandrakesoft.com>, Benoit PAPILLAULT <benoit.papillault@free.fr>

Source:		http://prdownloads.sourceforge.net/eciadsl/eciadsl-usermode-0.9.tar.gz

BuildRoot:	%_tmppath/%name-%version-%release
Requires: ppp
Prefix: /usr

%description
The eciadsl-usermode package contains the driver for the ADSL USB modem
called ECI Hi-Focus. It also support a lot of USB ADSL modems based upon
the Globespan chipset. It is not a kernel module, but a user-mode program
that handles the modem. A kernel module is under development.

%prep
# in this section, we remove old builds, untar the new sources and
# optionally make some patch. in our case, it only contains %setup
# which untar the sources
%setup -q

%build
# compile executable
	./configure --enable-rpm-maintainer-mode
	make -s ROOT="$RPM_BUILD_ROOT"

%install
	./configure --enable-rpm-maintainer-mode
	make -s ROOT="$RPM_BUILD_ROOT" install

%pre
# pre-install script

%post
# post-install script
echo "Now you need to configure the driver. Please read the README"
echo "and INSTALL files located in /usr/local/doc/eciadsl."

%preun
# pre-uninstall script

%postun
# post-uninstall script

%files
# here, we list all the files that form the binary package.
# executable files should be : rwxr-xr-x
%defattr(755,root,root)
%dir /usr/local/bin
/usr/local/bin/eciadsl-start
/usr/local/bin/eciadsl-stop
/usr/local/bin/eciadsl-firmware
/usr/local/bin/eciadsl-synch
/usr/local/bin/eciadsl-pppoeci
/usr/local/bin/eciadsl-doctor
/usr/local/bin/eciadsl-check-hdlc
/usr/local/bin/eciadsl-check-hdlc-bug
/usr/local/bin/eciadsl-config-tk
/usr/local/bin/eciadsl-config-text
/usr/local/bin/eciadsl-remove-dabusb
/usr/local/bin/eciadsl-probe-synch
/usr/local/bin/eciadsl-probe-device
/usr/local/bin/eciadsl-vendor-device.pl
/usr/local/bin/eciadsl-data.pl
/usr/local/bin/eciadsl-uc.pl
/usr/local/bin/eciadsl-makeconfig
%dir /usr/local/doc/eciadsl
/usr/local/doc/eciadsl/BUGS
/usr/local/doc/eciadsl/INSTALL
/usr/local/doc/eciadsl/INSTALL.fr
/usr/local/doc/eciadsl/README
/usr/local/doc/eciadsl/README.fr
/usr/local/doc/eciadsl/TODO
/usr/local/doc/eciadsl/TROUBLESHOOTING
/usr/local/doc/eciadsl/TROUBLESHOOTING.fr
# config files should be : rw-r--r--
%defattr(644,root,root)
%dir /etc/eciadsl
%config /etc/eciadsl/modems.db
%config /etc/eciadsl/providers.db
%config /etc/eciadsl/firmware00.bin
%config /etc/eciadsl/synch01.bin
%config /etc/eciadsl/modemeci.gif
# doc files should be : rw-r--r--
%defattr(644,root,root)
%doc README
%doc README.es
%doc README.fr
%doc README.it
%doc README.pt
%doc INSTALL
%doc INSTALL.es
%doc INSTALL.fr
%doc INSTALL.it
%doc INSTALL.pt
%doc TROUBLESHOOTING
%doc TROUBLESHOOTING.es
%doc TROUBLESHOOTING.fr
%doc TROUBLESHOOTING.it
%doc TROUBLESHOOTING.pt
%doc BUGS
%doc TODO
%doc rc.adsl

%clean
# in this section, we clean up temporary files generated during the build
	rm -rf %buildroot

%changelog
* Tue Nov 02 2004 Benoit PAPILLAULT <benoit.papillault@free.fr> 0.10-1
- Updated to the 0.10 release
* Sat Apr 03 2004 Benoit PAPILLAULT <benoit.papillault@free.fr> 0.9-1
- Updated to the 0.9 release
* Tue Jul 01 2003 Benoit PAPILLAULT <benoit.papillault@free.fr> 0.8-1
- Corrected a bug in reporting errors from the USB layer
* Wed Jan 22 2003 wwp <subscript@free.fr> 0.7-1
- last (?) fixes to 0.7.
* Tue Jan 21 2003 wwp <subscript@free.fr> 0.7-1
- made 0.7-ready.
* Tue Jan 21 2003 Benoit PAPILLAULT <benoit.papillault@free.fr> 0.6-2
- Allow non-root users to generate .rpm (removed the use of system rpm topdir).
* Sun Jan 05 2003 wwp <subscript@free.fr> 0.6-2
- Re-introduced ROOT for make invocation (fixes wrong relocation dir).
* Sat Jan 04 2003 wwp <subscript@free.fr> 0.6-1
- Updated for 0.6.
* Mon Apr 29 2002 Benoit PAPILLAULT <benoit.papillault@free.fr> 0.5-2
- Make a new spec file for 0.5. This spec file is in the CVS.
* Mon Dec 17 2001 David Faure <david@mandrakesoft.com> 0.2-1select.mdk
- Initial spec, from CVS sources (usermode and doc modules)
