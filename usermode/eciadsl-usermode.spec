# to build : rpm -ba eciadsl-usermode.spec
# or       : rpm -ta eciadsl-usermode-0.5.tar.gz
# the tar.gz should contain a directory eciadsl-usermode-0.5

Summary:	A beta-quality usermode driver for the ECI ADSL USB modem
Name:		eciadsl-usermode
Version:	0.5
Release:	2
License:	GPL
Group:		Networking/Other
Packager:	David Faure <david@mandrakesoft.com>, Benoit PAPILLAULT <benoit.papillault@free.fr>

Source:		http://prdownloads.sourceforge.net/eciadsl/eciadsl-usermode-0.5.tar.gz

BuildRoot:	%_tmppath/%name-%version-%release-root

%description
The eciadsl-usermode package contains the driver for the ADSL USB modem
called ECI Hi-Focus. It is not a kernel module, but a user-mode program
that handles the modem. A kernel module is under development.

%prep
# in this section, we remove old builds, untar the new sources and
# optionally make some patch. in our case, it only contains %setup
# which untar the sources
%setup -q

%build
# compile executable
	cd usermode
	make ROOT="$RPM_BUILD_ROOT"
# compile documentation (need latex, latex2html)
	cd ../doc
	make

%install
	cd usermode
	make ROOT="$RPM_BUILD_ROOT" install

%pre
# pre-install script

%post
# post-install script

# check-char-major-108:
	grep "^alias char-major-108" /etc/modules.conf > /dev/null 2>&1; \
	if [ $? -ne 0 ]; then \
		echo "adding ppp_generic alias ..." ; \
		echo "alias char-major-108 ppp_generic" >> /etc/modules.conf ; \
	fi
# check-tty-ldisc-14:
	grep "^alias tty-ldisc-14" /etc/modules.conf > /dev/null 2>&1; \
	if [ $? -ne 0 ]; then \
		echo "adding ppp_synctty alias ..." ;\
		echo "alias tty-ldisc-14 ppp_synctty" >> /etc/modules.conf ; \
	fi

# check-tty-ldisc-13:
	grep "^alias tty-ldisc-13" /etc/modules.conf > /dev/null 2>&1; \
	if [ $? -ne 0 ]; then \
		echo "adding n_hdlc alias ..." ;\
		echo "alias tty-ldisc-13 n_hdlc" >> /etc/modules.conf ; \
	fi

# install-etc-ppp-peers-adsl:
	if [ -f /etc/ppp/peers/adsl ]; then \
		echo "modifying existing /etc/ppp/peers/adsl" ; \
		user=`grep "^user " /etc/ppp/peers/adsl` ; \
		persist=`grep "^persist" /etc/ppp/peers/adsl` ; \
		grep -v "^user " /etc/eciadsl/adsl-skel > /etc/ppp/peers/adsl ; \
		echo $user >> /etc/ppp/peers/adsl ; \
		echo $persist >> /etc/ppp/peers/adsl ; \
	else \
		echo "creating /etc/ppp/peers/adsl" ; \
		cp /etc/eciadsl/adsl-skel /etc/ppp/peers/adsl ; \
		chmod 700 /etc/ppp/peers/adsl ; \
		chown root:root /etc/ppp/peers/adsl ; \
	fi

%preun
# pre-uninstall script

%postun
# post-uninstall script

%files
# here, we list all the files that form the binary package.
# executable files should be : rwxr-xr-x
%defattr(755,root,root)
%dir /usr/local/bin
/usr/local/bin/startmodem
/usr/local/bin/eci-load1
/usr/local/bin/eci-load2
/usr/local/bin/pppoeci
/usr/local/bin/eci-doctor.sh
/usr/local/bin/check-hdlc
/usr/local/bin/check-hdlc-bug
/usr/local/bin/eciconf.sh
/usr/local/bin/remove_dabusb
/usr/local/bin/makeconfig
# config files should be : rw-r--r--
%dir /etc/eciadsl
%defattr(644,root,root)
%config /etc/eciadsl/adsl-skel
%config /etc/eciadsl/eci_firm_kit_wanadoo.bin
%config /etc/eciadsl/eci_wan3.bin
%config /etc/eciadsl/eci_wan3.dmt.bin
%config /etc/eciadsl/modemeci.gif

# doc files
%defattr(-,root,root)
%doc doc/howto.txt
%doc doc/howto.tex
%doc doc/howto.ps
%doc doc/howto.dvi
%doc doc/howto.html
%doc doc/howto.css

%clean
# in this section, we clean up temporary files generated during the build
	rm -rf %buildroot

%changelog
* Mon Apr 29 2002 Benoit PAPILLAULT <benoit.papillault@free.fr> 0.5-2
- Make a new spec file for 0.5. This spec file is in the CVS.
* Mon Dec 17 2001 David Faure <david@mandrakesoft.com> 0.2-1select.mdk
- Initial spec, from CVS sources (usermode and doc modules)
