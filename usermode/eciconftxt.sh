#!/bin/sh
#
# ECI Linux driver configuration script.
# Same goal as eciconf.sh, but can be run in a terminal / console.
#
# Usage :
#   eciconftxt.sh <file.bin>   change quickly .bin file to sync
#   without parameter : interactive mode for configuration
#
# Author   : FlashCode <flashcode@flashtux.org>
# Creation : 2002/10/06
# Licence  : GPL
#

# <CONFIG>
BIN_DIR="/usr/local/bin"
ETC_DIR="/etc"
CONF_DIR="/etc/eciadsl"
PPPD_DIR="/etc/ppp"
# </CONFIG>

tmpbin=/tmp/binfile.tmp

if [ $UID -ne 0 ]; then
    echo -e "This script must be run as root.\nEnter the root password, please."
    su  - -c  "DISPLAY=$DISPLAY PATH=$PATH  $0  $*"
    exit 1
fi

case "$1" in
"@menu@")
    choice_ok=0
    echo $2 | grep ".*|.*|" >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        exit 255
    fi
    while [ $choice_ok -eq 0 ]; do
        item=$(echo $2 | cut -f 1 -d '|')
        echo -e "$item :\n"
        i=0
        fin=0
        possible="|"
        while [ $fin -eq 0 ]; do
            i=$(expr $i + 1)
            item=$(echo $2 | cut -f `expr $i + 1` -d '|')
            if [ -n "$item" ]; then
                echo "$i) $item"
                possible="${possible}$i|"
            else
                fin=1
            fi
        done
        i=$(expr $i - 1)
        echo -e -n "\nEnter your choice (1-$i) [default=1] : "
        read choice_menu
        if [ -z "$choice_menu" ]; then
            choice_menu=1
        fi
        echo $possible | grep "$choice_menu" >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            choice_ok=1
        else
            echo -e "* Error: Incorrect choice\n"
        fi
    done
    exit $choice_menu
	;;

"@bin@")
    fichiers_bin="Select your .bin file for sync|"
    echo
    find ${CONF_DIR} -type f -o -type l -name "*.bin" 2>/dev/null | grep "bin" >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "WARNING: no .bin file found in ${CONF_DIR} or subdirectories"
        echo "Please check your driver installation !"
        echo "Skipping .bin selection..."
        :> $tmpbin
        exit 255
    fi
    for bin in $(find ${CONF_DIR} -type f -name "*.bin"); do
		case "$bin" in
		*firmware*)	;;
		*)			test "$bin" != "${CONF_DIR}/synch.bin" && fichiers_bin="${fichiers_bin}$bin|";;
		esac
    done
    $0 @menu@ "$fichiers_bin"
	ret=$?
	test $ret -eq 255 && exit 255
    bin=$(expr $ret + 1)
    echo "$(echo $fichiers_bin | cut -f $bin -d '|')" > $tmpbin
	exit 0
	;;

"")
    menu1="==== What do you want to do|Make all configuration|Remove Dabusb module|Change .bin sync file"

    providers="Select your provider|Wanadoo|Club Internet|9 Telecom|Cegetel|World Online|Telecom Italia|Tiscali Italia|Pipex UK|Bluewin|Belgacom|Bezeq Intl.|Other|"
    servers_dns1="*|193.252.19.3|194.117.200.15|212.30.96.108|194.6.128.3|212.83.128.3|212.216.112.112|195.130.224.18|158.43.240.4|195.186.1.111|195.238.2.21|192.115.106.11|0.0.0.0|"
    servers_dns2="*|193.252.19.4|194.117.200.10|213.203.124.146|194.6.128.4|212.83.128.4|212.216.172.62|195.130.225.129|158.43.240.3|195.186.4.111|195.238.2.22|192.115.106.10|0.0.0.0|"

    modems="Select your modem|ECI Hifocus/B-Focus|Eicon Diva|Ericsson hm120dp|Aztech 100U|Fujitsu FDX310|US Robotics 8500|Webpower Ipmdatacom|BT Voyager|Wisecom ad80usg or EA100|Xentrix USB|Zyxel Prestige 630-41|Topcom Webr@cer 850|Siemens Santis|GVC BB039|Digicom MichelAngelo USB|Askey USB-ADSL Modem|Archtek UGW-8000|Cypress Globespan ADSL USB G7000"
    vid1pid1="*|05472131|071dac81|08ea00c9|05090801|0e600101|0baf00e6|09150001|16900203|09150001|0e600100|05472131|09150001|09150001|09150001|05472131|16900205|09150001|09150001"
    vid2pid2="*|09158000|0915ac82|091500ca|09150802|09150102|091500e7|09150002|09150204|09150002|09150101|09158000|09150002|09150002|09150002|09158000|09150206|09150002|09150002"

    echo -e "\n***** Welcome to Eci Adsl Linux driver configuration *****\n"
    echo -e "At any time, press Ctrl+C to quit this script without saving modifications.\n"
    echo -e "Tip: you can run eciconftxt.sh with a .bin file as parameter to change\nyour .bin quickly.\n"

    if [ ! -d $CONF_DIR ]; then
        echo -e "Config directory not found !\n"
        exit 1
    fi

    $0 @menu@ "$menu1"
	ret=$?
	test $ret -eq 255 && exit 1
    config=$ret

    ### Global Eci Adsl configuration ###

    if [ $config -eq 1 ]; then
        echo -e "\n===> Global Eci Adsl configuration <===\n"
        
        user=""
        while [ -z "$user" ]; do
            echo -n "Enter your user name (given by your provider) : "
            read user
        done
        pwdmatch=0
        while [ $pwdmatch -eq 0 ]; do
            stty -echo
            password=""
            while [ -z "$password" ]; do
                echo -n "Enter your password  (given by your provider) : "
                read password
                echo
            done
            password2=""
            while [ -z "$password2" ]; do
                echo -n "Enter your password again (for verification)  : "
                read password2
                echo
            done
            stty echo
            if [ "$password" == "$password2" ]; then
                pwdmatch=1
            else
                echo -e "\n* ERROR : Passwords don't match !"
            fi
        done
        
        echo
        $0 @menu@ "$providers"
		ret=$?
		test $ret -eq 255 && exit 1
        provider=$(expr $ret + 1)
        
        echo
        echo "If your provider was not listed (or if you want to overwrite defaults),"
        echo "you can enter your own DNS servers :"
        dns1=""
        choice_ok=0
        while [ $choice_ok -eq 0 ]; do
            echo -n "DNS server 1 (in case of doubt simply press Enter) : "
            read dns1
            if [ -z "$dns1" ]; then
                choice_ok=1
            else
                echo $dns1 | grep -E "^([0-9]{1,3}\.){3}[0-9]{1,3}$" >/dev/null 2>&1
                if [ $? -eq 0 ]; then
                    choice_ok=1
                else
                    echo "Invalid DNS 1. Please enter it again."
                fi
            fi
        done
        dns2=""
        choice_ok=0
        while [ $choice_ok -eq 0 ]; do
            echo -n "DNS server 2 (in case of doubt simply press Enter) : "
            read dns2
            if [ -z "$dns2" ]; then
                choice_ok=1
            else
                echo $dns2 | grep -E "^([0-9]{1,3}\.){3}[0-9]{1,3}$" >/dev/null 2>&1
                if [ $? -eq 0 ]; then
                    choice_ok=1
                else
                    echo "Invalid DNS 2. Please enter it again."
                fi
            fi
        done
        
        if [ -z "$dns1" ]; then
            dns1=$(echo $servers_dns1 | cut -f $provider -d '|')
        fi
        if [ -z "$dns2" ]; then
            dns2=$(echo $servers_dns2 | cut -f $provider -d '|')
        fi
        
        echo
        echo "Enter now your VPI/VCI (depending on your provider/country)"
        echo "Example for France: 8 35  (VPI=8, VCI=35)"
        echo "These values correspond to the number dialed under Windows."
        vpi=""
        while [ -z "$vpi" ]; do
            echo -n "Enter your VPI (given by your provider, example: 8)  : "
            read vpi
        done
        vci=""
        while [ -z "$vci" ]; do
            echo -n "Enter your VCI (given by your provider, example: 35) : "
            read vci
        done
        
        echo
        $0 @menu@ "$modems"
		ret=$?
		test $ret -eq 255 && exit 1
        modem=$(expr $ret + 1)
        
        $0 @bin@
		ret=$?
        binfile=$(cat $tmpbin)
        rm -rf $tmpbin >/dev/null

        echo
        echo "==== Configuration will be created with these values :"
        echo
        echo "  + User        : $user"
        echo "  + Password    : (hidden)"
        echo "  + Provider    : $(echo $providers | cut -f $provider -d '|')"
        echo "      DNS 1     : $dns1"
        echo "      DNS 2     : $dns2"
        echo "  + VPI/VCI     : $vpi/$vci"
        echo "  + Modem       : $(echo $modems | cut -f $modem -d '|')"
        echo "      VID1/PID1 : $(echo $vid1pid1 | cut -f $modem -d '|' | cut -c 1-4)/$(echo $vid1pid1 | cut -f $modem -d '|' | cut -c 5-8)"
        echo "      VID2/PID2 : $(echo $vid2pid2 | cut -f $modem -d '|' | cut -c 1-4)/$(echo $vid2pid2 | cut -f $modem -d '|' | cut -c 5-8)"
        echo "  + .bin file   : $binfile"
        echo
        echo "Press Enter to create config files or Ctrl+C now to exit now without saving."
        read quitte

        $BIN_DIR/makeconfig "$user" "$password" /usr/local/bin/pppoeci $dns1 $dns2 $vpi $vci \
			$(echo $vid1pid1 | cut -f $modem -d '|') $(echo $vid2pid2 | cut -f $modem -d '|') "$binfile"

        if [ $? -eq 0 ]; then
            echo
            echo "==== eciconftxt.sh: Configuration updated with success !"
            echo "You can now run startmodem (as root) to connect to the internet."
        else
            echo "ERROR: makeconfig reports error(s), please fix the problem and run eciconftxt.sh again."
        fi
        echo
    elif [ $config -eq 2 ]; then
        echo
        echo "Please unplug your modem and then press Enter."
        read pause
        remove_dabusb
        echo
    else
        $0 @bin@
		ret=$?
        binfile=$(cat $tmpbin)
        rm -rf $tmpbin >/dev/null
		test $ret -eq 255 && exit 1
        echo
        $0 $binfile
        echo
    fi
	;;

*)
    # .bin change
    if [ ! -f "$1" ]; then
        echo "ERROR: .bin file \"$1\" does not exist !"
		exit 1
    fi
    if [ -L "$1" ]; then
        echo "ERROR: .bin file \"$1\" is a symbolic link !"
        exit 1
    fi
	synch_bin_link="${CONF_DIR}/synch.bin"
    echo -n "Modifying .bin link ($synch_bin_link -> $1)..."
    rm -f "$synch_bin_link" >/dev/null
    ln -sf "$1" "$synch_bin_link"
    echo "ok"
	;;
esac
exit 0
