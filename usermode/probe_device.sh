#!/bin/sh

# probe device's VIDs and PIDs
VERSION="0.0.1"


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
	echo "          --batch <device name>  batch mode (non interactive)"
	echo "          --version              show version number then exit"
	echo "          --help                 show this help then exit"
	exit $1
}


BASE=${0##*/}
BIN=eci-load1
DEVICES=/proc/bus/usb/devices
VID1="????"
PID1="????"
VID2="????"
PID2="????"
DEVICE=""
declare -i TESTONLY=0 BATCHMODE=0

while [ -n "$1" ]
do
	case "$1" in
		"--dry-run")	let TESTONLY=1;;
		"--batch")		let BATCHMODE=1
						if [ -n "$2" ]
						then
							shift
							DEVICE="$@"
							break
						else
							echo "missing arguments for switch $1"
							commandlinehelp 1
						fi;;
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
	test $BATCHMODE -eq 0 && echo -e "\nyou must be root to run this script!"
	exit 1
fi

# disclaimer
if [ $BATCHMODE -eq 0 ]
then
	echo -e "\nWARNING: before probing, please ensure that your USB devices are plugged"
	echo "in and that your system's USB support is properly configured"
fi

if [ ! -f "$DEVICES" ]
then
	test $BATCHMODE -eq 0 && echo -e "\nUSB devices file not found!"
	exit 1
fi

# list USB devices
if [ $BATCHMODE -eq 0 ]
then
	echo -e "\nYour USB devices:"
	BUFFER=$(grep -i product "$DEVICES")
	declare -i LINES=0
	OLDIFS=$IFS
	IFS=$'\n'
	for LINE in $BUFFER
	do
	let LINES+=1
	echo -n "$LINES: "
	echo $LINE | cut -d '=' -f 2-
	done
	if [ $LINES -lt 1 ]
	then
		echo "no USB device found!"
		exit 1
	fi

	# prompt for a device
	echo -en "\nenter device to probe (1-$LINES): "
	read DEVNUM
	while [ $DEVNUM -lt 1 -o $DEVNUM -gt $LINES ]
	do
		echo "incorrect value"
		echo -en "\nenter device to probe (1-$LINES): "
		read DEVNUM
	done

	# get device
	let LINES=0
	DEVICE=""
	for LINE in $BUFFER
	do
		let LINES+=1
		if [ $LINES -eq $DEVNUM ]
		then
			DEVICE="$(echo $LINE | cut -d '=' -f 2-)"
			break
		fi
	done
	IFS=$OLDIFS
else
	grep -B2 "$DEVICE" "$DEVICES"  > /dev/null 2>&1 || exit 1
fi

# get IDs
BUFFER=$(grep -B2 "$DEVICE" "$DEVICES" | grep -i vendor | tr -s ' ' '=')
if [ $? -ne 0 ]
then
	test $BATCHMODE -ne 0 && exit 1
else
	VID1=$(echo $BUFFER | cut -d '=' -f 3)
	PID1=$(echo $BUFFER | cut -d '=' -f 5)
fi

type $BIN > /dev/null 2>&1
if [ $? -ne 0 ]
then
	test $BATCHMODE -eq 0 && echo -e "\ncannot find $BIN in \$PATH, test mode assumed"
	let TESTONLY=1
fi

if [ $TESTONLY -eq 0 ]
then
	eci-load1 0x$VID1 0x$PID1 0xDEAD 0xFACE > /dev/null 2>&1
	BUFFER=$(grep -B2 "$DEVICE" "$DEVICES" | grep -i vendor | tr -s ' ' '=')
	if [ $? -ne 0 ]
	then
		test $BATCHMODE -ne 0 && exit 1
	else
		VID2=$(echo $BUFFER | cut -d '=' -f 3)
		PID2=$(echo $BUFFER | cut -d '=' -f 5)
	fi
fi

# display results
test $BATCHMODE -eq 0 && echo -e "\nprobed USB device: $DEVICE"
if [ $BATCHMODE -eq 0 ]
then 
	echo "VID1=$VID1, PID1=$PID1"
	echo "VID2=$VID2, PID2=$PID2"
else
	echo "$VID1 $PID1 $VID2 $PID2"
fi
exit 0
