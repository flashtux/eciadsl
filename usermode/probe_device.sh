#!/bin/sh

# probe device's VIDs and PIDs
VERSION="0.0.5"


function version()
{
	echo "$VERSION"
	exit 0
}

function commandlinehelp()
{
	echo "usage:"
	echo "          $BASE [<switch>..]"
	echo "switches:"
	echo "          --dry-run              test mode (only perform neutral operations)"
	echo "          --version              show version number then exit"
	echo "          --help                 show this help then exit"
	echo "          --smart                use smart mode: only show uninitialized devices"
	echo "          --auto                 device auto selection"
	exit $1
}

function display_device()
{
	test -n "$PRODUCT" && echo -n " $PRODUCT" || echo -n " ?"
	test -n "$MANUFACTURER" && echo -n "$MANUFACTURER"
	test -n "$ID" && echo " ($ID)" || echo " (?$SEP?)"
	test -n "$ID_TABLE" && ID_TABLE="$ID_TABLE|$ID" || ID_TABLE="$ID"
	test -n "$PRODUCT_TABLE" && PRODUCT_TABLE="$PRODUCT_TABLE|$PRODUCT" || PRODUCT_TABLE="$PRODUCT"
	test -n "$MANUFACTURER_TABLE" && MANUFACTURER_TABLE="$MANUFACTURER_TABLE|$MANUFACTURER" || MANUFACTURER_TABLE="$MANUFACTURER"
}

function list_devices()
{
	let CNT=0
	ID_TABLE=""
	PRODUCT_TABLE=""
	MANUFACTURER_TABLE=""
	local ID=""
	local PRODUCT=""
	local MANUFACTURER=""
	local OLDIFS=$IFS
	IFS=$'\n'
	for LINE in $(cat $DEVICES)
	do
		case ${LINE:0:2} in
		"T:")	
				test $CNT -ne 0 && display_device
				let CNT+=1
				echo -n "$CNT: "
				ID=""
				PRODUCT=""
				MANUFACTURER=""
				;;
		"P:")	
				if [ -z "$ID" ]
				then
					ID=$(echo "$LINE" | grep -i "prodid=" | awk '{print $2,$3}')
					if [ $? -eq 0 -a -n "$ID" ]
					then
						ID=$(echo "$ID" | tr -s ' ' '=' | cut -d '=' -f 2,4 | tr -s '=' $SEP)
					fi
				fi
				;;
		"S:")	
				if [ -z "$PRODUCT" ]
				then
					PRODUCT=$(echo "$LINE" | grep -i "product=" | cut -d '=' -f 2-)
				fi
				if [ -z "$MANUFACTURER" ]
				then
					MANUFACTURER=$(echo "$LINE" | grep -i "manufacturer=" | cut -d '=' -f 2-)
					if [ $? -eq 0 -a -n "$MANUFACTURER" ]
					then
						MANUFACTURER=" / $MANUFACTURER"
					fi
				fi
				;;
		esac
	done
	IFS=$OLDIFS
	test $CNT -ne 0 && display_device
	if [ $CNT -lt 1 ]
	then
		echo "no USB device found!"
		exit 1
	fi
}

function get_ID()
{
	OLDIFS=$IFS
	IFS="|"
	let CNT=0
	ID=""
	for LINE in $ID_TABLE
	do
		let CNT+=1
		if [ $CNT -eq $DEVNUM ]
		then
			test -n "$LINE" && ID="$LINE"
			break
		fi
	done
	IFS=$OLDIFS
}

function get_product()
{
	OLDIFS=$IFS
	IFS="|"
	let CNT=0
	PRODUCT="?"
	for LINE in $PRODUCT_TABLE
	do
		let CNT+=1
		if [ $CNT -eq $DEVNUM ]
		then
			test -n "$LINE" && PRODUCT="$LINE"
			break
		fi
	done
	IFS=$OLDIFS
}

function get_manufacturer()
{
	OLDIFS=$IFS
	IFS="|"
	let CNT=0
	MANUFACTURER=""
	for LINE in $MANUFACTURER_TABLE
	do
		let CNT+=1
		if [ $CNT -eq $DEVNUM ]
		then
			test -n "$LINE" && MANUFACTURER="$LINE"
			break
		fi
	done
	IFS=$OLDIFS
}

# <CONFIG>
BIN_DIR="/usr/local/bin"
ETC_DIR="/etc"
CONF_DIR="/etc/eciadsl"
PPPD_DIR="/etc/ppp"
# </CONFIG>

BASE=${0##*/}
BIN=$BIN_DIR/eci-load1
FIRMWARE=$CONF_DIR/firmware.bin
DEVICES=/proc/bus/usb/devices
VID1_TABLE=""
VID2_TABLE=""
PID1_TABLE=""
PID2_TABLE=""
VID1="????"
PID1="????"
VID2="????"
PID2="????"
SEP=":"
declare -i TESTONLY=0 RET=0 CNT SMART=0 AUTO=0

while [ -n "$1" ]
do
	case "$1" in
		"--dry-run")	let TESTONLY=1;;
		"--smart")		let SMART=1;;
		"--auto")		let AUTO=1;;
		"--version")	version;;
		"--help")		commandlinehelp 0;;
		*)				echo "unrecognized switch $1"
						commandlinehelp 1
						;;
	esac
	shift
done

if [ $UID -ne 0 ]
then
	echo -e "\nyou must be root to run this script!"
	exit 1
fi

# disclaimer
echo -e "\nWARNING: before probing, please ensure that your USB devices are plugged in"
echo "and that your system's USB support is properly configured"
echo -e "\nUSB modem to probe must be UNinitialized, it will surely appear as an unknown"
echo "device (because it is not initialized yet), for instance: ? (0547:2131)"

if [ ! -f "$DEVICES" ]
then
	echo -e "\nUSB devices file not found!"
	exit 1
fi

# list USB devices
echo -e "\nyour USB devices:"
list_devices

# prompt for a device number
echo -en "\nenter device to probe (1-$CNT): "
read DEVNUM
while [ $DEVNUM -lt 1 -o $DEVNUM -gt $CNT ]
do
	echo "incorrect value"
	echo -en "\nenter device to probe (1-$LINES): "
	read DEVNUM
done

# get device info from table
get_ID
get_product
get_manufacturer

# get VID1/PID1
if [ ${#ID} -eq 9 -a "${ID:4:1}" == "$SEP" ]
then
	VID1=${ID:0:4}
	PID1=${ID:5:4}
else
	echo -e "\ncannot determine VID1/PID1 for device $PRODUCT$MANUFACTURER ($ID)"
	exit 1
fi

type $BIN > /dev/null 2>&1
if [ $? -ne 0 ]
then
	echo -e "\ncannot find $BIN in \$PATH, test mode assumed"
	let RET+=1
#	let TESTONLY=1
fi

if [ $TESTONLY -eq 0 ]
then
	echo -e "\nprobing, please wait.."
	$BIN 0x$VID1 0x$PID1 0xDEAD 0xFACE $FIRMWARE > /dev/null 2>&1

	# list USB devices
	echo -e "\nyour USB devices now:"
	list_devices

	# get device info from table
	get_ID
	get_product
	get_manufacturer

	if [ ${#ID} -eq 9 -a "${ID:4:1}" == "$SEP" ]
	then
		VID2=${ID:0:4}
		PID2=${ID:5:4}
	else
		echo -e "\ncannot determine VID2/PID2 for device $PRODUCT$MANUFACTURER ($ID)"
		let RET+=1
	fi
fi

# display results
echo -e "\nprobed USB device: $PRODUCT$MANUFACTURER"
echo "VID1=$VID1, PID1=$PID1"
echo "VID2=$VID2, PID2=$PID2"
if [ "$VID1" == "$VID2" -a "$PID1" == "$PID2" ]
then
	echo "Did you really unplug/replug your modem before launching this script?"
	let RET+=1
fi

exit $RET
