#!/bin/sh

# Author: Benoit PAPILLAULT <benoit.papillault@free.fr>
# Creation: 01/02/2002
# Licence: GPL

# 02/02/2002: added the permission check for check-hdlc & check-hdlc-bug
# 07/02/2002: added check on the pppd output. added the CVS Id.

# CVS $Id$
# Tag $Name$

# 16/02/2002 Benoit PAPILLAULT
#   Check for an existing pppd and display its version

# 26/02/2002 Benoit PAPILLAULT
#   Added the 'nopersist' flag for diagnostic purpose.

# <CONFIG>
ETC_DIR=/etc
CONF_DIR=/etc/eciadsl
BIN_DIR=/usr/local/bin
PPPD_DIR=/etc/ppp
# </CONFIG>

# cd to the directory where the 'doctor' is.
cd `dirname $0`

PATH=/bin:/sbin:/usr/bin:/usr/sbin

if [ `whoami` != "root" ]; then
	echo "You need to be root (type su)" ;
	exit -1;
fi

# display the kernel version
echo "You are using linux kernel version `uname -r`"

# note the size in lines of /var/log/messages
line=`wc -l /var/log/messages | awk '{print $1}'`

function fatal () {
	tail +$line /var/log/messages > /tmp/msg.log
# check for "usb-uhci.c: ENXIO 80000380, flags 0, urb c7f401c0, burb c6469140"
	grep 'usb-uhci.c: ENXIO' /tmp/msg.log

# check for "usb-uhci.c: interrupt, status 29, frame# 956"
	grep 'usb-uhci.c: interrupt' /tmp/msg.log

# check for "usb_control/bulk_msg: timeout"
	grep 'usb_control/bulk_msg' /tmp/msg.log

# check for "usb-uhci.c: Host controller halted, trying to restart"
	grep 'usb-uhci.c: Host controller halted' /tmp/msg.log

# check for "usb-uhci.c: process_transfer: fixed toggle"
	grep 'usb-uhci.c: process_transfer' /tmp/msg.log

# check for "usb-uhci.c: iso_find_start: gap in seamless isochronous scheduling"
	grep 'usb-uhci.c: iso_find_start' /tmp/msg.log
	rm /tmp/msg.log
	exit -1;
}

if [ ! -d /proc/bus/usb ]; then
	echo "Support for USB is missing... trying to load" ;
	modprobe usbcore
	sleep 1
	if [ ! -d /proc/bus/usb ]; then
		echo "Support for USB: failed to load" ;
		fatal;
	else
		echo "Support for USB is OK" ;
	fi
else
	echo "Support for USB is OK" ;
fi

# mount usbdevfs is this is not the case
if [ ! -f /proc/bus/usb/devices ]; then
	echo "Preliminary USB device filesystem is missing... trying to mount" ;
	mount -t usbdevfs none /proc/bus/usb
	sleep 1
	if [ ! -f /proc/bus/usb/devices ]; then
		echo "Preliminary USB device filesystem: failed to load" ;
		fatal ;
	else
		echo "Preliminary USB device filesystem is OK" ;
	fi
else
	echo "Preliminary USB device filesystem is OK" ;
fi

# check for the dabusb module
lsmod | grep "^dabusb" > /dev/null
if [ $? -eq 0 ]; then
	echo "dabusb module is loaded: trying to unload!" ;
	modprobe -r dabusb
	sleep 1
	lsmod | grep "^dabusb" > /dev/null
	if [ $? -eq 0 ]; then
		echo "dabusb module cannot be unloaded, verify if it is busy";
		fatal;
	else
		echo "dabusb module unloaded: OK";
	fi
else
	echo "dabusb module is not loaded: OK";
fi

# check for the ehci-hcd module
lsmod | grep "^ehci-hcd" > /dev/null
if [ $? -eq 0 ]; then
	echo "ehci-hcd module is loaded: trying to unload!" ;
	modprobe -r ehci-hcd
	sleep 1
	lsmod | grep "^ehci-hcd" > /dev/null
	if [ $? -eq 0 ]; then
		echo "ehci-hcd module cannot be unloaded, verify if it is busy";
		fatal;
	else
		echo "ehci-hcd module unloaded: OK";
	fi
else
	echo "ehci-hcd module is not loaded: OK";
fi

# try to locate UHCI controller
uhci=0
# directly check for the UHCI driver (avoid to use /proc/pci if there is none)
grep "^S:  Product=USB UHCI Root Hub" /proc/bus/usb/devices > /dev/null
if [ $? -ne 0 ]; then
	if [ -f /proc/pci ]; then
		grep -A 4 USB /proc/pci | grep I/O > /dev/null
		if [ $? -eq 0 ]; then
		# ok, we have a UHCI controller, check if the linux driver is loaded
			grep "^S:  Product=USB UHCI Root Hub" /proc/bus/usb/devices > /dev/null
			if [ $? -ne 0 ]; then
				echo "UHCI support is missing... trying to load" ;
				modprobe usb-uhci
				modprobe uhci
				sleep 2
				grep "^S:  Product=USB UHCI Root Hub" /proc/bus/usb/devices > /dev/null
				if [ $? -ne 0 ]; then
					echo "UHCI support: failed to load" ;
				else
					echo "UHCI support is OK" ;
					uhci=1 ;
				fi
			else
				echo "UHCI support is OK" ;
				uhci=1 ;
			fi
		fi
	else
		echo "You don't have /proc/pci. I cannot check for a UHCI controller";
	fi
else
	echo "UHCI support is OK" ;
	uhci=1;
fi

# try to locate OHCI controller
ohci=0
grep "^S:  Product=USB OHCI Root Hub" /proc/bus/usb/devices > /dev/null
if [ $? -ne 0 ]; then
	if [ -f /proc/pci ]; then
		grep -A 4 USB /proc/pci | grep memory > /dev/null
		if [ $? -eq 0 ]; then
		# ok, we have a OHCI controller, check if the linux driver is loaded
			grep "^S:  Product=USB OHCI Root Hub" /proc/bus/usb/devices > /dev/null
			if [ $? -ne 0 ]; then
				echo "OHCI support is missing... trying to load" ;
				modprobe usb-ohci
				sleep 2
				grep "^S:  Product=USB OHCI Root Hub" /proc/bus/usb/devices > /dev/null
				if [ $? -ne 0 ]; then
					echo "OHCI support: failed to load" ;
				else
					echo "OHCI support is OK" ;
					ohci=1 ;
				fi
			else
				echo "OHCI support is OK" ;
				ohci=1 ;
			fi
		fi
    else
		echo "You don't have /proc/pci. I cannot check for a OHCI controller";
	fi
else
	echo "OHCI support is OK" ;
	ohci=1;
fi

if [ $uhci -eq 0 -a $ohci -eq 0 ]; then
	echo "I found no USB controller" ;
	fatal;
fi

# check for the presense of /dev/ppp
if [ ! -c /dev/ppp ]; then
	echo "/dev/ppp is missing... trying to create" ;
	mknod /dev/ppp c 108 0
	if [ ! -c /dev/ppp ]; then
		echo "/dev/ppp: failed to create" ;
		fatal;
	fi
fi

# check some property of /dev/ppp
set `ls -la /dev/ppp`
if [ $3 != "root" ]; then
	echo "/dev/ppp should be owned by root... trying to change" ;
	chown root /dev/ppp
	set `ls -la /dev/ppp`
	if [ $3 != "root" ]; then
		echo "/dev/ppp: failed to change owner to root" ;
		fatal;
	fi
fi

if [ $5 != "108," ]; then
	echo "/dev/ppp has a bad major number... trying to change" ;
	rm /dev/ppp
	mknod /dev/ppp c 108 0
	set `ls -la /dev/ppp`
	if [ $5 != "108," ]; then
		echo "/dev/ppp: failed to change major number" ;
		fatal;
	fi
fi

if [ $6 != "0" ]; then
	echo "/dev/ppp has a bad minor number... trying to change" ;
	rm /dev/ppp
	mknod /dev/ppp c 108 0
	set `ls -la /dev/ppp`
	if [ $6 != "0" ]; then
		echo "/dev/ppp: failed to change minor number" ;
		fatal;
	fi
fi
echo "/dev/ppp is OK" ;

# check if the 'check-hdlc' file is present and executable
# this is important since sometime downloading a file loose the
# executable bit (via DCC for instance)
if [ -f check-hdlc -a ! -x check-hdlc ]; then
	chmod a+x check-hdlc ;
fi

# check for the HDLC support
./check-hdlc
if [ $? -ne 0 ]; then
	echo "HDLC support is missing... trying to load" ;
	modprobe n_hdlc ;
	./check-hdlc ;
	if [ $? -ne 0 ]; then
		echo "HDLC support: failed to load" ;
		echo "You should check your kernel config with: cd /usr/src/linux ; make menuconfig" ;
		echo "and look under Character devices for Non-standard serial port support and" ;
		echo "HDLC line discipline support" ;
		fatal;
	fi
# here, HDLC support is OK, but maybe some alias are missing
	modprobe -r n_hdlc ;
	./check-hdlc ;
	if [ $? -ne 0 ]; then
		echo "HDLC support: alias is missing... trying to add" ;
		echo "alias tty-ldisc-13 n_hdlc" >> $(ETC_DIR)/modules.conf ;
		depmod -a ;
# checking again
		./check-hdlc ;
		if [ $? -ne 0 ]; then
			echo "HDLC support: adding alias does not work" ;
			fatal;
		else
			echo "HDLC support is OK" ;
		fi
	else
		echo "HDLC support is OK" ;
	fi
else
	echo "HDLC support is OK" ;
fi

# check if the 'check-hdlc-bug' is present and executable
# this is important since sometime downloading a file loose the
# executable bit (via DCC for instance)
if [ -f check-hdlc-bug -a ! -x check-hdlc-bug ]; then
	chmod a+x check-hdlc-bug
fi

# check for the HDLC bug
./check-hdlc-bug > /dev/null 2>&1
if [ $? -ne 0 ]; then
	echo "HDLC support is buggy, you should apply the HDLC patch to your \
kernel source. See the howto in the doc directory or on \
http://eciadsl.sourceforge.net/ in the Documentation section for further \
instructions." ;
else
	echo "HDLC support is OK (no bug)" ;
fi

if [ -e $CONF_DIR/vidpid ]; then
    vid1=`cat $CONF_DIR/vidpid | cut -f1 -d' '` ;
    pid1=`cat $CONF_DIR/vidpid | cut -f2 -d' '` ;
    vid2=`cat $CONF_DIR/vidpid | cut -f3 -d' '` ;
    pid2=`cat $CONF_DIR/vidpid | cut -f4 -d' '` ;
    echo "vid/pid read from $CONF_DIR/vidpid: $vid1/$pid1 $vid2/$pid2" ;
else
    vid1="0547" ;
    pid1="2131" ;
    vid2="0915" ;
    pid2="8000" ;
fi

# check for the EZUSB chip
ezusb=0
grep "^P:  Vendor=$vid1 ProdID=$pid1" /proc/bus/usb/devices > /dev/null
if [ $? -eq 0 ]; then
	ezusb=1 ;
	echo "Loading EZ-USB firmware... ";
	$BIN_DIR/eci-load1 0x$vid1 0x$pid1 0x$vid2 0x$pid2 $CONF_DIR/firmware.bin
	if [ $? -ne 0 ] ; then
		echo "Failed to load EZ-USB firmware" ;
		fatal ;
	fi ;
	echo "Loading the GlobeSpan firmware..." ;
	$BIN_DIR/eci-load2 0x$vid2 0x$pid2 $CONF_DIR/synch.bin
	if [ $? -ne 0 ] ; then
		echo "Failed to load GlobeSpan firmware" ;
		fatal ;
	fi ;
fi

# check for the $PPPD_DIR/peers/adsl file (if the user is using another name,
# he knows what he's doing and does not need this script).

if [ ! -f $PPPD_DIR/peers/adsl ]; then
	echo "No $PPPD_DIR/peers/adsl: did you install the ECI driver?" ;
	fatal;
fi

# Note: user option (either in $PPPD_DIR/peers/adsl or $PPPD_DIR/chap-secrets)
# can be :
# user TheUser
# user "TheUser"
# user 'TheUser'

# check for an existing user param. The actual user param may be
# within "" or not.
user=`grep "^user" $PPPD_DIR/peers/adsl | awk '{print $2}'`
if [ "$user" = "" ]; then
	echo "Option 'user' if missing from $PPPD_DIR/peers/adsl: Fatal" ;
	fatal ;
fi

# remove "" and '' from $user if needed.
n=`echo $user | cut -d'"' -f2` ;
if [ "$n" != "" ]; then
	user=$n;
fi
n=`echo $user | cut -d'"' -f2`;
if [ "$n" != "" ]; then
	user=$n;
fi

# check that the user param is the same is $PPPD_DIR/peers/adsl and
# $PPPD_DIR/chap-secrets

grep "^$user[ \t]*" $PPPD_DIR/chap-secrets > /dev/null
if [ $? -ne 0 ]; then
	grep "^'$user'[ \t]*" $PPPD_DIR/chap-secrets > /dev/null
	if [ $? -ne 0 ]; then
		grep "^\"$user\"[ \t]*" $PPPD_DIR/chap-secrets > /dev/null
		if [ $? -ne 0 ]; then
			echo "$PPPD_DIR/chap-secrets: no password for $user" ;
			echo "Give me the password for $user:"
			read pwd
			echo "$user * $pwd *" >> $PPPD_DIR/chap-secrets
		fi
	fi
else
	echo "$PPPD_DIR/chap-secrets is OK" ;
fi

# check for the Globespan chip
grep "^P:  Vendor=$vid2 ProdID=$pid2" /proc/bus/usb/devices > /dev/null
if [ $? -ne 0 ]; then
	echo "I cannot find your ADSL modem: Fatal"
	fatal;
fi

# check for an existing pppd
x=`which pppd`
if [ "$x" = "" ]; then
	echo "No pppd is intalled: Fatal" ;
	exit -1;
fi

# check the pppd version
ppp_version=`pppd --version | cut -d" " -f 3` ;
msg="";
if [ "$ppp_version" != "2.4.0" -a "$ppp_version" != "2.4.1" ]; then
	msg=" (untested)" ;
fi
echo "You are using pppd version $ppp_version$msg" ;

# check for an existing $PPPD_DIR/options file
if [ -f $PPPD_DIR/options ]; then
	echo "You have an $PPPD_DIR/options file. Options in this file may conflict with" ;
	echo "options from $PPPD_DIR/peers/adsl. We suggest to remove this file or make a";
	echo "backup copy." ;
	grep "^nodetach" $PPPD_DIR/options > /dev/null
	if [ $? -eq 0 ]; then
		echo "Removing 'nodetach' option from $PPPD_DIR/options..." ;
		grep -v "^nodetach" $PPPD_DIR/options > /tmp/options
		mv /tmp/options $PPPD_DIR/options
	fi
fi

# check for an existing PPP connection (select the first one if several)
PPP=`ifconfig | grep "^ppp" | head -1 | awk '{print $1}'`
if [ "$PPP" = "" ]; then
	echo "No existing PPP connection... trying to make one (please wait)" ;
	nice --20 pppd call adsl updetach nopersist | tee /tmp/ppp.log

# check if we succeed in making a new PPP connection
	PPP=`ifconfig | grep "^ppp" | head -1 | awk '{print $1}'`
	if [ "$PPP" = "" ]; then
		# check for usermode driver crash
		grep "Modem hangup" /tmp/ppp.log > /dev/null
		if [ $? -eq 0 ]; then
			echo "PPP: very bad ... usermode driver just crashed" ;
			rm /tmp/ppp.log
			fatal;
		fi
		# check for no response from PPP
		grep 'LCP: timeout sending Config-Requests' /tmp/ppp.log > /dev/null
		if [ $? -eq 0 ]; then
			echo "PPP connection failed: check your vci & vpi parameters in $PPPD_DIR/peers/adsl and check for USB errors in /var/log/messages" ;
			rm /tmp/ppp.log
			fatal;
		fi
		# check for invalid password
		grep 'CHAP authentication failed' /tmp/ppp.log > /dev/null
		if [ $? -eq 0 ]; then
			echo "CHAP authentication failed: check your user in $PPPD_DIR/peers/adsl and the matching password in $PPPD_DIR/chap-secrets" ;
			rm /tmp/ppp.log
			fatal;
		fi
		# check for "sent [LCP ConfRej id=0xa5 <auth chap MD5>]"
		grep 'sent \[LCP ConfRej' /tmp/ppp.log | grep '<auth chap MD5>' > /dev/null
		if [ $? -eq 0 ]; then
			echo "Password for user $user is missing in $PPPD_DIR/chap-secrets";
			rm /tmp/ppp.log
			fatal;
		fi
		echo "Cannot make a PPP connection: Fatal" ;
		rm /tmp/ppp.log
		fatal;
	else
		rm /tmp/ppp.log
		echo "PPP connection is OK" ;
    fi
else
	echo "PPP connection is OK" ;
fi

# check for the default route over pppN
route -n | grep "^0.0.0.0" | grep $PPP > /dev/null
if [ $? -ne 0 ]; then
	echo "No default route over $PPP... trying to add" ;
	route add default dev $PPP
	route -n | grep "^0.0.0.0" | grep $PPP > /dev/null
	if [ $? -ne 0 ]; then
		echo "No default over $PPP: failed" ;
		fatal;
	else
		echo "Default route over $PPP is OK" ;
    fi
else
	echo "Default route over $PPP is OK" ;
fi

# check for the default route not over ethN
route -n | grep "^0.0.0.0" | grep -v $PPP > /dev/null
if [ $? -eq 0 ]; then
	echo "You have default route(s) not over $PPP... trying to delete" ;
	other=`route -n | grep "^0.0.0.0" | grep -v $PPP | awk '{print $8}'`;
	for itf in $other;
	do
		echo "Deleting default route over $itf" ;
		route del default dev $itf ;
	done
	route -n | grep "^0.0.0.0" | grep -v $PPP > /dev/null
	if [ $? -eq 0 ]; then
		echo "Deleting default route not over $PPP: failed" ;
		fatal;
    fi
fi

# check if ICMP packets are sent & received (is it usefull?)
#ping -c 10 yahoo.fr > /dev/null
#if [ $? -eq 0 ]; then
#	echo "ICMP traffic is OK" ;
#else
#	echo "ICMP traffic does not work" ;
#fi

# check for /var/log or /tmp partitions full (TODO)
# check for "rcvd [LCP TermReq id=0xa8]" (TODO)
# check for $ETC_DIR/resolv.conf validity (TODO)
# check for an already running pppd & pppoeci, while no ppp0 (TODO)
# check for $PPPD_DIR/pap-secrets too (TODO)
# check for either pap-secrets or chap-secrets, how? (TODO)

echo "Everything is OK" ;

