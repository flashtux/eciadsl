#!/bin/sh

# Author: Benoit PAPILLAULT <benoit.papillault@free.fr>
# Creation: 01/02/2002

# 02/02/2002: added the permission check for check-hdlc & check-hdlc-bug

if [ `whoami` != "root" ]; then
	echo "You need to be root (type su)" ;
	exit -1;
fi

if [ ! -d /proc/bus/usb ]; then
	echo "Support for USB is missing... trying to load" ;
	/sbin/modprobe usbcore
	if [ ! -d /proc/bus/usb ]; then
		echo "Support for USB: failed to load" ;
		exit -1;
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
	if [ ! -f /proc/bus/usb/devices ]; then
		echo "Preliminary USB device filesystem: failed to load" ;
		exit -1 ;
	else
		echo "Preliminary USB device filesystem is OK" ;
	fi
else
	echo "Preliminary USB device filesystem is OK" ;
fi

# try to locate UHCI controller
uhci=0
grep -A 4 USB /proc/pci | grep I/O > /dev/null
if [ $? -eq 0 ]; then
# ok, we have a UHCI controller, check if the linux driver is loaded
	grep "^S:  Product=USB UHCI Root Hub" /proc/bus/usb/devices > /dev/null
	if [ $? -ne 0 ]; then
		echo "UHCI support is missing... trying to load" ;
		/sbin/modprobe usb-uhci
		/sbin/modprobe uhci
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

# try to locate OHCI controller
ohci=0
grep -A 4 USB /proc/pci | grep memory > /dev/null
if [ $? -eq 0 ]; then
# ok, we have a OHCI controller, check if the linux driver is loaded
	grep "^S:  Product=USB OHCI Root Hub" /proc/bus/usb/devices > /dev/null
	if [ $? -ne 0 ]; then
		echo "OHCI support is missing... trying to load" ;
		/sbin/modprobe usb-ohci
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

if [ $uhci -eq 0 -a $ohci -eq 0 ]; then
	echo "I found no USB controller" ;
	exit -1;
fi

# check for the presense of /dev/ppp
if [ ! -c /dev/ppp ]; then
	echo "/dev/ppp is missing... trying to create" ;
	mknod /dev/ppp c 108 0
	if [ ! -c /dev/ppp ]; then
		echo "/dev/ppp: failed to create" ;
		exit -1;
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
		exit -1;
	fi
fi

if [ $5 != "108," ]; then
	echo "/dev/ppp has a bad major number... trying to change" ;
	/bin/rm /dev/ppp
	mknod /dev/ppp c 108 0
	set `ls -la /dev/ppp`
	if [ $5 != "108," ]; then
		echo "/dev/ppp: failed to change major number" ;
		exit -1;
	fi
fi

if [ $6 != "0" ]; then
	echo "/dev/ppp has a bad minor number... trying to change" ;
	/bin/rm /dev/ppp
	mknod /dev/ppp c 108 0
	set `ls -la /dev/ppp`
	if [ $6 != "0" ]; then
		echo "/dev/ppp: failed to change minor number" ;
		exit -1;
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
	/sbin/modprobe n_hdlc ;
	./check-hdlc ;
	if [ $? -ne 0 ]; then
		echo "HDLC support: failed to load" ;
		echo "You should check your kernel config with: cd /usr/src/linux ; make menuconfig" ;
		echo "and look under Character devices for Non-standard serial port support and" ;
		echo "HDLC line discipline support" ;
		exit -1;
	fi
# here, HDLC support is OK, but maybe some alias are missing
	/sbin/rmmod n_hdlc ;
	./check-hdlc ;
	if [ $? -ne 0 ]; then
		echo "HDLC support: alias is missing... trying to add" ;
		echo "alias tty-ldisc-13 n_hdlc" >> /etc/modules.conf ;
		depmod -a ;
# checking again
		./check-hdlc ;
		if [ $? -ne 0 ]; then
			echo "HDLC support: adding alias does not work" ;
			exit -1;
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

# check for the HDLC bug (TODO)
./check-hdlc-bug > /dev/null 2>&1
if [ $? -ne 0 ]; then
	echo "HDLC support is buggy, you should apply the HDLC patch to your \
kernel source. See the howto in the doc directory or on \
http://eciadsl.sourceforge.net/ in the Documentation section for further \
instructions." ;
else
	echo "HDLC support is OK (no bug)" ;
fi

# check for the EZUSB chip
ezusb=0
grep "^P:  Vendor=0547 ProdID=2131" /proc/bus/usb/devices > /dev/null
if [ $? -eq 0 ]; then
	ezusb=1 ;
	echo "Loading EZ-USB firmware... ";
	/usr/local/bin/eci-load1 /usr/local/bin/eci_firm_kit_wanadoo.bin
	if [ $? -ne 0 ] ; then
		echo "Failed to load firmware" ;
		exit -1 ;
	fi ;
fi

# check for the /etc/ppp/peers/adsl file (if the user is using another name,
# he knows what he's doing and does not need this script).

if [ ! -f /etc/ppp/peers/adsl ]; then
	echo "No /etc/ppp/peers/adsl: did you install the ECI driver?" ;
	exit -1;
fi

# check for an existing user param. The actual user param may be 
# within "" or not.
user=`grep "^user" /etc/ppp/peers/adsl | awk '{print $2}'`
if [ "$user" = "" ]; then
	echo "Option 'user' if missing from /etc/ppp/peers/adsl: Fatal" ;
	exit -1 ;
fi

# remove "" from $user if needed.
o=`echo $user | awk -F'"' '{print $2}'` ;
if [ "$o" != "" ]; then
	user=$o ;
fi

# check that the user param is the same is /etc/ppp/peers/adsl and
# /etc/ppp/chap-secrets

grep "^$user[ \t]*" /etc/ppp/chap-secrets > /dev/null
if [ $? -ne 0 ]; then
	echo "/etc/ppp/chap-secrets: no password for $user" ;
	echo "Give me the password for $user:"
	read pwd
	echo "$user * $pwd *" >> /etc/ppp/chap-secrets
else
	echo "/etc/ppp/chap-secrets is OK" ;
fi

# check for the Globespan chip
grep "^P:  Vendor=0915 ProdID=8000" /proc/bus/usb/devices > /dev/null
if [ $? -ne 0 ]; then
	echo "I cannot find your ADSL modem: Fatal"
	exit -1;
fi

# check for an existing PPP connection (select the first one if several)
PPP=`/sbin/ifconfig | grep "^ppp" | head -1 | awk '{print $1}'`
if [ "$PPP" = "" ]; then
	echo "No existing PPP connection... trying to make one" ;
	nice --20 pppd call adsl updetach > /tmp/ppp.log
fi

# check for the default route over pppN (TODO)
/sbin/route -n | grep "^0.0.0.0" | grep $PPP > /dev/null
if [ $? -ne 0 ]; then
	echo "No default route over $PPP... trying to add" ;
	/sbin/route add default dev $PPP
	/sbin/route -n | grep "^0.0.0.0" | grep $PPP > /dev/null
	if [ $? -ne 0 ]; then
		echo "No default over $PPP: failed" ;
		exit -1;
	else
		echo "Default route over $PPP is OK" ;
    fi
else
	echo "Default route over $PPP is OK" ;
fi

# check for the default route not over ethN (TODO)
/sbin/route -n | grep "^0.0.0.0" | grep -v $PPP > /dev/null
if [ $? -eq 0 ]; then
	echo "You have default route(s) not over $PPP... trying to delete" ;
	other=`/sbin/route -n | grep "^0.0.0.0" | grep -v $PPP | awk '{print $8}'`;
	for itf in $other;
	do
		echo "Deleting default route over $itf" ;
		/sbin/route del default dev $itf ;
	done
	/sbin/route -n | grep "^0.0.0.0" | grep -v $PPP > /dev/null
	if [ $? -eq 0 ]; then
		echo "Deleting default route not over $PPP: failed" ;
		exit -1;
    fi
fi

# check for /var/log or /tmp partitions full (TODO)
# check for USB interrupt in /var/log/messages (TODO)
# check for no response from PPP (TODO)
# check for "sent [LCP ConfRej id=0xa5 <auth chap MD5>]" (TODO)
# check for "rcvd [LCP TermReq id=0xa8]" (TODO)
# check for the dabusb module (TODO)
# check for an installed pppd (/usr/sbin/pppd) (TODO)

echo "Everything is OK" ;

