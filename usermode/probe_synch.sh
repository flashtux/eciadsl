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

echo -e "\nWARNING: if no $CONF_DIR/vidpid file exists, default vid/pid"
echo "will be assumed. This vidpid file is generated using eciconf.sh"
echo "or eciconftxt.sh."
echo "this script requires your modem to be supported by the driver,"
echo "and that you've installed some extra synch .bin (the official"
echo "synch_bin package for instance)"


if [ $INTERACTIVE -eq 1 ]
then
	echo -e "\nenter directory where to find the synch .bin"
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

if [ -f "$CONF_DIR/vidpid" ]; then
	VID1=`cat "$CONF_DIR/vidpid" | cut -f 1 -d ' '`
	PID1=`cat "$CONF_DIR/vidpid" | cut -f 2 -d ' '`
	VID2=`cat "$CONF_DIR/vidpid" | cut -f 3 -d ' '`
	PID2=`cat "$CONF_DIR/vidpid" | cut -f 4 -d ' '`
	echo -e "\ngot vid/pid from $CONF_DIR/vidpid"
else
	VID1="0547"
	PID1="2131"
	VID2="0915"
	PID2="8000"
	echo -e "\ndefault vid/pid assumed"
fi
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
