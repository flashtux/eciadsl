#!/bin/sh

# probe device's VIDs and PIDs


function version()
{
	echo "$VERSION"
	exit 0
}

function commandlinehelp()
{
	echo "usage:"
	echo "          $BASE [<switch>..] [dir]"
	echo "switches:"
	echo "          --try-all              try all the synch .bin even if one is OK"
	echo "          --batch                disable default interactive mode"
	echo "          --version or -v        show version number then exit"
	echo "          --help or -h           show this help then exit"
	exit $1
}


# <CONFIG>
BIN_DIR="/usr/local/bin"
ETC_DIR="/etc"
CONF_DIR="/etc/eciadsl"
PPPD_DIR="/etc/ppp"
VERSION=""
# </CONFIG>

BASE=${0##*/}
BIN1="$BIN_DIR/eci-load1"
BIN2="$BIN_DIR/eci-load2"
FIRMWARE="$CONF_DIR/firmware.bin"
DIR=""
declare -i INTERACTIVE=1 ALL=0

while [ -n "$1" ]
do
	case "$1" in
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
	echo -e "\nyou must be root to run this script!"
	exit 1
fi

test -z "$DIR" && DIR=$CONF_DIR

echo -e "\nWARNING: if no $CONF_DIR/eciadsl.conf file exists,"
echo "default vid/pid will be assumed. This eciadsl.conf file is generated using"
echo "eciconf.sh or eciconftxt.sh"
echo "this script requires your modem to be supported by the driver,"
echo "and that you've installed some extra synch .bin (the official"
echo "synch_bin package for instance)"


if [ $INTERACTIVE -eq 1 ]
then
	echo -e "\ntype in a directory where to find the synch .bin"
	echo -en "[default is $DIR]: "
	read USERDIR
	test -n "$USERDIR" && DIR="$USERDIR"
fi

if [ ! -d "$DIR" ]
then
	echo -e "\ncannot access to $DIR"
	exit 1
fi

LIST=""
for FILE in $(ls "$DIR")
do
	if [ -f "$DIR/$FILE" -a ! -L "$DIR/$FILE" -a -s "$DIR/$FILE" ]
	then
		case "$FILE" in
		*firm*)		;;
		*".bin")	test -n "$LIST" && LIST="$LIST\n$FILE" || LIST="$FILE";;
		*)			;;
		esac
	fi
done

if [ -z "$LIST" ]
then
	echo -e "\nno synch .bin has been found in $DIR, quitting"
	exit 1
fi

echo -e "\nthese synch .bin will be tested:"
echo -e "$LIST"
if [ $INTERACTIVE -eq 1 ]
then
	echo -en "start the tests now? (Y/n) "
	read CONFIRM
	case $CONFIRM in
	Y|y|"")	;;
	*)		echo "cancelled by user, exiting"
			exit 1;;
	esac
fi

if [ -f "$CONF_DIR/eciadsl.conf" ]; then
	VID1=`grep -iE "^[ \t]*VID1[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t" `
	PID1=`grep -iE "^[ \t]*PID1[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t" `
	VID2=`grep -iE "^[ \t]*VID2[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t" `
	PID2=`grep -iE "^[ \t]*PID2[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t" `
	echo -e "\ngot vid/pid from $CONF_DIR/eciadsl.conf"
else
	echo -e "\ndefault vid/pid assumed"
fi
test -z "$VID1" && VID1="0547"
test -z "$PID1" && PID1="2131"
test -z "$VID2" && VID2="0915"
test -z "$PID2" && PID2="8000"

echo -e "loading firmware.."
echo $BIN1 0x$VID1 0x$PID1 0x$VID2 0x$PID2 "$FIRMWARE" 

$BIN1 0x$VID1 0x$PID1 0x$VID2 0x$PID2 "$FIRMWARE" > /dev/null 2>&1
if [ $? -eq 0 ]
then
	echo "firmware loaded"
else
	echo "failed to load firmware, aborting"
	exit 1
fi

OLDIFS="$IFS"
IFS="
"
for FILE in $(echo -e "$LIST")
do
	echo -e "\ntesting synch with $DIR/$FILE.."
	$BIN2 0x$VID2 0x$PID2 "$DIR/$FILE"
	if [ $? -eq 0 ]
	then
		test -n "$LISTOK" && LISTOK="$LISTOK\n$FILE" || LISTOK="$FILE"
		echo "$DIR/$FILE seems OK"
		test $ALL -eq 0 && break
	fi
	echo "trying the next one.."
	sleep 1
done
IFS="$OLDIFS"

if [ -z "$LISTOK" ]
then
	echo -e "\nno valid synch .bin has been found. There are many possible issues"
	echo "and it may be necessary to generate your own one. Please refer to"
	echo "the documentation in both cases."
else
	echo -e "\nthese synch .bin are supposed to be OK:"
	echo -e "$LISTOK"
fi

exit
