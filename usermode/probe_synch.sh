#!/bin/sh

# probe device's VIDs and PIDs


function version()
{
	echo "$VERSION"
	exit 0
}

function commandlinehelp()
{
	echo "Usage:"
	echo "          $BASE [<switch>..] [dir]"
	echo "Switches:"
	echo "          --try-all              try all the synch .bin even if one is OK"
	echo "          --full-init            reset modem between each synch .bin test"
	echo "          --batch                disable default interactive mode"
	echo "          --version or -v        show version number then exit"
	echo "          --help or -h           show this help then exit"
	exit $1
}

function reset_usb()
{
#		modprobe -r $HUB > /dev/null
#		sleep 1
#		modprobe $HUB > /dev/null
		sleep 1
}

function init_modem()
{
	echo -e "\nLoading firmware.."
	$BIN_DIR/eci-load1 $ECILOAD1_OPTIONS 0x$VID1 0x$PID1 0x$VID2 0x$PID2 "$FIRMWARE" > /dev/null 2>&1
	if [ $? -eq 0 ]
	then
		echo "Firmware loaded"
	else
		echo "*** failed to load firmware, aborting"
		exit 1
	fi
}


# <CONFIG>
BIN_DIR="/usr/local/bin"
ETC_DIR="/etc"
CONF_DIR="/etc/eciadsl"
PPPD_DIR="/etc/ppp"
VERSION=""
# </CONFIG>

BASE=${0##*/}
DIR=""
declare -i ALL=0 INTERACTIVE=1 FULL_INIT=0

while [ -n "$1" ]
do
	case "$1" in
		"--full_init")		let FULL_INIT=1;;
		"--try-all")		let ALL=1;;
		"--batch")			let INTERACTIVE=0;;
		"--version"|"-v")	version;;
		"--help"|"-h")		commandlinehelp 0;;
		*)					test -z "$DIR" && DIR=$1 || break;;
	esac
	shift
done

if [ $UID -ne 0 ]
then
	echo -e "\nYou must be root to run this script!"
	exit 1
fi


echo -e "\nWARNING: if no $CONF_DIR/eciadsl.conf file exists,"
echo "default vid/pid will be assumed. This eciadsl.conf file is generated using"
echo "eciconf.sh or eciconftxt.sh"
echo -e "\nThis script requires your modem to be supported by the driver,"
echo "and that you've installed some extra synch .bin (the official"
echo "synch_bin package for instance)"

ECILOAD1_OPTIONS=""
ECILOAD2_OPTIONS=""
if [ -f "$CONF_DIR/eciadsl.conf" ]; then
	VID1=`grep -iE "^[ \t]*VID1[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	PID1=`grep -iE "^[ \t]*PID1[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	VID2=`grep -iE "^[ \t]*VID2[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	PID2=`grep -iE "^[ \t]*PID2[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	FIRMWARE=`grep -iE "^[ \t]*FIRMWARE[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -s "\t" " "`
	ECILOAD1_OPTIONS=`grep -iE "^[ \t]*ECILOAD1_OPTIONS[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -s " \t" " "`
	ECILOAD2_OPTIONS=`grep -iE "^[ \t]*ECILOAD2_OPTIONS[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -s " \t" " "`
	echo -e "\nConfig read from $CONF_DIR/eciadsl.conf"
else
	echo -e "\nDefault config assumed"
fi
test -z "$VID1" && VID1="0547"
test -z "$PID1" && PID1="2131"
test -z "$VID2" && VID2="0915"
test -z "$PID2" && PID2="8000"
test -z "$FIRMWARE" && FIRMWARE="$CONF_DIR/firmware00.bin"
test -z "$DIR" && DIR=$CONF_DIR

if [ $INTERACTIVE -eq 1 ]
then
	echo -e "\nType in a directory where to find the synch .bin"
	echo -en "[default is $DIR]: "
	read USERDIR
	test -n "$USERDIR" && DIR="$USERDIR"
fi

if [ ! -d "$DIR" ]
then
	echo -e "\n*** cannot access to $DIR"
	exit 1
fi

LIST=""
for FILE in $(find "$DIR" -maxdepth 1 -type f -name "*.bin" | grep -v firm)
do
	test -n "$LIST" && LIST="$LIST\n$FILE" || LIST="$FILE";;
done

if [ -z "$LIST" ]
then
	echo -e "\n*** no synch .bin has been found in $DIR, aborting"
	exit 1
fi

echo -e "\nThese synch .bin will be tested:"
echo -e "$LIST"
if [ $INTERACTIVE -eq 1 ]
then
	echo -en "Start the tests now? (Y/n) "
	read CONFIRM
	case $CONFIRM in
	Y|y|"")	;;
	*)		echo "*** cancelled by user, exiting"
			exit 1;;
	esac
fi

# HUB=usb-uhci

if [ $FULL_INIT -eq 0 ]
then
	init_modem
else
	reset_usb
fi

OLDIFS="$IFS"
IFS="
"
for FILE in $(echo -e "$LIST")
do
	if [ $FULL_INIT -eq 1 ]
	then
		init_modem
	fi

	echo -e "\nTesting synch with $DIR/$FILE.."
	$BIN_DIR/eci-load2 $ECILOAD2_OPTIONS 0x$VID2 0x$PID2 "$DIR/$FILE"
	if [ $? -eq 0 ]
	then
		test -n "$LISTOK" && LISTOK="$LISTOK\n$FILE" || LISTOK="$FILE"
		echo "$DIR/$FILE seems OK"
		test $ALL -eq 0 && break
	fi
	echo "Trying the next one.."
	sleep 1

	if [ $FULL_INIT -eq 1 ]
	then
		reset_usb
	fi
done
IFS="$OLDIFS"

if [ -z "$LISTOK" ]
then
	echo -e "\n** No valid synch .bin has been found"
	echo "There are many possible issues"
	echo "and it may be necessary to generate your own one. Please refer to"
	echo "the documentation in both cases."
else
	echo -e "\nThese synch .bin are supposed to be OK:"
	echo -e "$LISTOK"
fi
exit
