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
VERSION=""
# </CONFIG>

if [ "$1" == "--version" -o "$1" == "-v" ]; then
	echo "$VERSION"
	exit 0
fi

tmpbin=/tmp/binfile.tmp
previous="Previous setting"

if [ $UID -ne 0 ]; then
    echo -e "This script must be run as root.\nEnter the root password, please."
    su  - -c  "DISPLAY=$DISPLAY PATH=$PATH  $0  $*"
    exit 1
fi

if [ -s "$CONF_DIR/eciadsl.conf" ]; then
	prev_vpi=`grep -E "^[ \t]*VPI[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	prev_vci=`grep -E "^[ \t]*VCI[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	prev_vid1=`grep -E "^[ \t]*VID1[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	prev_pid1=`grep -E "^[ \t]*PID1[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	prev_vid2=`grep -E "^[ \t]*VID2[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	prev_pid2=`grep -E "^[ \t]*PID2[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	prev_mode=`grep -E "^[ \t]*MODE[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	prev_firmware=`grep -E "^[ \t]*FIRMWARE[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -s "\t" " "`
	prev_synch=`grep -E "^[ \t]*SYNCH[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -s "\t" " "`
	prev_staticip=`grep -E "^[ \t]*STATICIP[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	prev_gateway=`grep -E "^[ \t]*GATEWAY[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	prev_use_dhcp=`grep -E "^[ \t]*USE_DHCP[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -d " \t"`
	prev_modem=`grep -E "^[ \t]*MODEM[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -s "\t" " "`
	prev_provider=`grep -E "^[ \t]*PROVIDER[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -s "\t" " "`
	prev_pppd_user=`grep -E "^[ \t]*PPPD_USER[ \t]*=" "$CONF_DIR/eciadsl.conf" | tail -1 | cut -f 2 -d '=' | tr -s "\t" " "`
fi

case "$1" in
"@menu@")
    choice_ok=0
    echo $2 | grep -E "^[^|]+(\|[^|]+)*\|[^|]+$" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
		echo -e "*** internal error: invalid menu\n$2"
        exit 255
    fi
    while [ $choice_ok -eq 0 ]; do
        item=$(echo $2 | cut -f 1 -d '|')
        echo -e "$item:\n"
        i=0
        fin=0
        possible="|"
        while [ $fin -eq 0 ]; do
            i=$(expr $i + 1)
            item=$(echo $2 | cut -f `expr $i + 1` -d '|')
            if [ -n "$item" ]; then
                echo "$i) $item"
                possible="$possible$i|"
            else
                fin=1
            fi
        done
        i=$(expr $i - 1)
        echo -en "\nEnter your choice (1-$i, default is 1): "
        read choice_menu
        if [ -z "$choice_menu" ]; then
            choice_menu=1
        fi
        echo $possible | grep "$choice_menu" > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            choice_ok=1
        else
            echo -e "Incorrect choice, try again\n"
        fi
    done
    exit $choice_menu
	;;

"@bin@")
    echo
	fichiers_bin=""
    for bin in $(find "$CONF_DIR" -type f -name "*.bin" | grep -v "firmware"); do
		fichiers_bin="$fichiers_bin|$bin"
    done
    if [ -z "$fichiers_bin" ]; then
        echo "WARNING: no synch .bin file found in $CONF_DIR or subdirectories"
        echo "Please check your driver installation!"
		if [ -n "$prev_synch" ]; then
	        echo "Previous selection kept"
			echo "$prev_synch" > $tmpbin
			exit 0
		else
	        echo "Skipping synch .bin selection.."
	        :> $tmpbin
	        exit 255
		fi
    fi
	if [ -n "$prev_synch" ]; then
	    fichiers_bin="|$previous ($prev_synch)$fichiers_bin"
	fi
    fichiers_bin="Select your .bin file for synch$fichiers_bin"
    $0 @menu@ "$fichiers_bin"
	ret=$?
	test $ret -eq 255 && exit 255
    bin=$(expr $ret + 1)
    echo "$(echo $fichiers_bin | cut -f $bin -d '|')" > $tmpbin
	exit 0
	;;

"")
    menu1="Your choice|Configure all settings|Remove dabusb module|Change synch .bin file"

    if [ ! -d $CONF_DIR ]; then
        echo -e "Config directory not found!\n"
        exit 1
    fi

	OLDIFS="$IFS"
	IFS="
"
	providers_db="$CONF_DIR/providers.db"
    providers="Select your provider"
    servers_dns1="*"
    servers_dns2="*"
	if [ -n "$prev_provider" ]; then
		providers="$providers|$previous ($prev_provider)"
		servers_dns1="$servers_dns1|$(grep -v -E "^[ \t]*#" "$providers_db" | grep "$prev_provider" | tr -s "\t" "|" | cut -d '|' -f 2)"
		servers_dns2="$servers_dns2|$(grep -v -E "^[ \t]*#" "$providers_db" | grep "$prev_provider" | tr -s "\t" "|" | cut -d '|' -f 3)"
	fi
	for LINE in $(grep -v -E "^[ \t]*#" "$providers_db" | tr -s "\t" "|" | cut -d '|' -f 1); do
		providers="$providers|$LINE"
	done
	for LINE in $(grep -v -E "^[ \t]*#" "$providers_db" | tr -s "\t" "|" | cut -d '|' -f 2); do
		servers_dns1="$servers_dns1|$LINE"
	done
	for LINE in $(grep -v -E "^[ \t]*#" "$providers_db"| tr -s "\t" "|" | cut -d '|' -f 3); do
		servers_dns2="$servers_dns2|$LINE"
	done
    providers="$providers|Other"
    servers_dns1="$servers_dns1|?"
    servers_dns2="$servers_dns2|?"

	modems_db="$CONF_DIR/modems.db"
    modems="Select your modem"
    vid1pid1="*"
    vid2pid2="*"
	if [ -n "$prev_modem" -a -n "$prev_vid1" -a -n "$prev_pid1" -a -n "$prev_vid2" -a -n "$prev_pid2" ]; then
		modems="$modems|$previous ($prev_modem)"
		vid1pid1="$vid1pid1|$prev_vid1$prev_pid1"
		vid2pid2="$vid2pid2|$prev_vid2$prev_pid2"
	fi
	for LINE in $(grep -v -E "^[ \t]*#" "$modems_db" | tr -s "\t" "|" | cut -d "|" -f 1); do
		modems="$modems|$LINE"
	done
	for LINE in $(grep -v -E "^[ \t]*#" "$modems_db" | tr -s "\t" "|" | cut -d "|" -f 2-3 | tr -d "| "); do
		vid1pid1="$vid1pid1|$LINE"
	done
	for LINE in $(grep -v -E "^[ \t]*#" "$modems_db" | tr -s "\t" "|" | cut -d "|" -f 4-5 | tr -d "| "); do
		vid2pid2="$vid2pid2|$LINE"
	done
	IFS="$OLDIFS"
    modems="$modems|Other"
    vid1pid1="$vid1pid1|00000000"
    vid2pid2="$vid2pid2|00000000"

    echo -e "\n***** Welcome to Eci Adsl Linux driver configuration *****\n"
    echo -e "At any time, press Ctrl+C to quit this script without saving modifications.\n"
    echo -e "Tip: you can run eciconftxt.sh with a .bin file as parameter to change\nyour .bin quickly.\n"

    $0 @menu@ "$menu1"
	ret=$?
	test $ret -eq 255 && exit 1
    config=$ret

    ### Global Eci Adsl configuration ###

    if [ $config -eq 1 ]; then
        echo -e "\n===> Eci Adsl driver setup <===\n"

        user=""
        while [ -z "$user" ]; do
			if [ -n "$prev_pppd_user" ]; then
				echo "Current user name is: $prev_pppd_user"
				TMP=", press ENTER to keep the previous one"
			else
				TMP=""
			fi
            echo -n "Type in your user name (given by your provider$TMP): "
            read user
			if [ -z "$user" -a -n "$prev_pppd_user" ]; then
				user="$prev_pppd_user"
				echo "User name unchanged"
			fi
        done

		echo
        pwdmatch=0
        while [ $pwdmatch -eq 0 ]; do
            stty -echo
            password=""
            while [ -z "$password" ]; do
                echo -n "Type in your password (given by your provider): "
                read password
                echo
            done
            password2=""
            while [ -z "$password2" ]; do
                echo -n "Type in your password again (for verification): "
                read password2
                echo
            done
            stty echo
            if [ "$password" == "$password2" ]; then
                pwdmatch=1
            else
                echo -e "Passwords don't match, try again\n"
            fi
        done

        echo
        $0 @menu@ "$providers"
		ret=$?
		test $ret -eq 255 && exit 1
        provider=$(expr $ret + 1)
        dns1="$(echo $servers_dns1 | cut -f $provider -d '|')"
        dns2="$(echo $servers_dns2 | cut -f $provider -d '|')"

        echo
        echo "If your provider was not listed (or if you want to overwrite"
        echo "defaults), you can type in your own DNS servers:"
		echo

		echo -n "DNS1 currently "
		if [ "$dns1" == "?" ]; then
			echo "unset"
			TMP="skip"
		else
			echo "set to $dns1"
			TMP="keep current value"
		fi
        i_dns1=""
        while [ -z "$i_dns1" ]; do
            echo -n "Type in an IP for DNS1 (press ENTER to $TMP): "
            read i_dns1
            if [ -z "$i_dns1" ]; then
				if [ "$dns1" == "?" ]; then
	                dns1=""
				else
					echo "DNS1 unchanged"
				fi
				break
            else
                echo $i_dns1 | grep -E "^([0-9]{1,3}\.){3}[0-9]{1,3}$" > /dev/null 2>&1
                if [ $? -ne 0 ]; then
					i_dns1=""
                    echo -e "Invalid IP for DNS1, please retry\n"
                else
					if [ "$i_dns1" != "$dns1" ]; then
						provider="Other"
						dns1="$i_dns1"
					fi
                fi
            fi
        done

		echo
        i_dns2=""
		echo -n "DNS2 currently "
		if [ "$dns2" == "?" ]; then
			echo "unset"
			TMP="skip"
		else
			echo "set to $dns2"
			TMP="keep current value"
		fi
        while [ -z "$i_dns2" ]; do
            echo -n "Type in an IP for DNS2 (press ENTER to $TMP): "
            read i_dns2
            if [ -z "$i_dns2" ]; then
				if [ "$dns2" == "?" ]; then
	                dns2=""
				else
					echo "DNS2 unchanged"
				fi
				break
            else
                echo $i_dns2 | grep -E "^([0-9]{1,3}\.){3}[0-9]{1,3}$" > /dev/null 2>&1
                if [ $? -ne 0 ]; then
					i_dns2=""
                    echo -e "Invalid IP for DNS2, please retry\n"
                else
					if [ "$i_dns2" != "$dns2" ]; then
						provider="Other"
						dns2="$i_dns2"
					fi
                fi
            fi
        done

        echo
        echo "Enter now your VPI/VCI (depending on your provider/country)"
        echo "Example for France: 8 35  (VPI=8, VCI=35)"
        echo "These values correspond to the number dialed under Windows."
		echo
		echo -n "VPI currently "
		if [ -z "$prev_vpi" ]; then
			echo "unset"
			TMP=""
		else
			echo "set to $prev_vpi"
			TMP=", press ENTER to keep the previous one"
		fi
        vpi=""
        while [ -z "$vpi" ]; do
	        echo -n "Type in your VPI (given by your provider$TMP): "
            read vpi
			if [ -n "$prev_vpi" -a -z "$vpi" ]; then
				vpi="$prev_vpi"
				echo "VPI unchanged"
			else
            	echo $vpi | grep -E "^[0-9]+$" > /dev/null 2>&1
            	if [ $? -ne 0 ]; then
                	vpi=""
                	echo -e"Invalid VPI, please enter it again\n"
            	fi
			fi
        done
		echo
		echo -n "VCI currently "
		if [ -z "$prev_vci" ]; then
			echo "unset"
			TMP=""
		else
			echo "set to $prev_vci"
			TMP=", press ENTER to keep the previous one"
		fi
        vci=""
        while [ -z "$vci" ]; do
	        echo -n "Type in your VPI (given by your provider$TMP): "
            read vci
			if [ -n "$prev_vci" -a -z "$vci" ]; then
				vci="$prev_vci"
				echo "VCI unchanged"
			else
	            echo $vci | grep -E "^[0-9]+$" > /dev/null 2>&1
            	if [ $? -ne 0 ]; then
                	vci=""
                	echo -e "Invalid VCI, please enter it again\n"
            	fi
			fi
        done

        echo
        $0 @menu@ "$modems"
		ret=$?
		test $ret -eq 255 && exit 1
        modem=$(expr $ret + 1)
		vid1="$(echo $vid1pid1 | cut -f $modem -d '|' | cut -c 1-4)"
		pid1="$(echo $vid1pid1 | cut -f $modem -d '|' | cut -c 5-8)"
		vid2="$(echo $vid2pid2 | cut -f $modem -d '|' | cut -c 1-4)"
		pid2="$(echo $vid2pid2 | cut -f $modem -d '|' | cut -c 5-8)"

		echo
        echo "If your modem was not listed (or if you want to overwrite defaults),"
        echo "you can enter your own VID1/PID1/VID2/PID2 (use probe_device.sh to"
		echo "get them):"

		echo
		echo "VID1 currently set to $vid1"
        i_vid1=""
        while [ -z "$i_vid1" ]; do
            echo -n "Type in a VID1 (4-digit hexadecimal, press ENTER to keep current value): "
            read i_vid1
            if [ -z "$i_vid1" ]; then
				echo "VID1 unchanged"
                break
            else
                echo $i_vid1 | grep -E "^([0-9A-Fa-f]{4})$" > /dev/null 2>&1
                if [ $? -eq 0 ]; then
					vid1="$i_vid1"
                else
					i_vid1=""
                    echo -e "Invalid VID1, please enter it again\n"
                fi
            fi
        done

		echo
		echo "PID1 currently set to $pid1"
		i_pid1=""
        while [ -z "$i_vid1" ]; do
            echo -n "Type in a PID1 (4-digit hexadecimal, press ENTER to keep current value): "
            read i_pid1
            if [ -z "$i_pid1" ]; then
				echo "PID1 unchanged"
                break
            else
                echo $i_pid1 | grep -E "^([0-9A-Fa-f]{4})$" > /dev/null 2>&1
                if [ $? -eq 0 ]; then
					pid1="$i_pid1"
                else
					i_pid1=""
                    echo -e "Invalid PID1, please enter it again\n"
                fi
            fi
        done

		echo
		echo "VID2 currently set to $vid2"
        i_vid2=""
        while [ -z "$i_vid2" ]; do
            echo -n "Type in a VID2 (4-digit hexadecimal, press ENTER to skip): "
            read i_vid2
            if [ -z "$i_vid2" ]; then
				echo "VID2 unchanged"
				break
            else
                echo $i_vid2 | grep -E "^([0-9A-Fa-f]{4})$" > /dev/null 2>&1
                if [ $? -eq 0 ]; then
					vid2="$i_vid2"
                else
					i_vid2=""
                    echo -e "Invalid VID2, please enter it again\n"
                fi
            fi
        done

		echo
		echo "PID2 currently set to $pid2"
        i_pid2=""
        while [ -z "$i_pid2" ]; do
            echo -n "Type in a PID2 (4-digit hexadecimal, press ENTER to skip): "
            read i_pid2
            if [ -z "$i_pid2" ]; then
				echo "PID2 unchanged"
                break
            else
                echo $i_pid2 | grep -E "^([0-9A-Fa-f]{4})$" > /dev/null 2>&1
                if [ $? -eq 0 ]; then
					pid2="$i_pid2"
                else
					i_pid2=""
                    echo -e "Invalid PID2, please enter it again\n"
                fi
            fi
        done

        $0 @bin@
		ret=$?
        binfile=$(cat $tmpbin)
        rm -rf $tmpbin > /dev/null

		OLDIFS="$IFS"
		IFS="
"
		modes="Select your PPP mode (use the DEFAULT one if you don't know what this means)"
		if [ -n "$prev_mode" ]; then
			modes="$modes|$previous ($prev_mode)"
		fi
		for LINE in $($BIN_DIR/pppoeci --modes 2>&1 | cut -d ' ' -f 1); do
			modes="$modes|$LINE"
		done
	    modes="$modes"
		IFS="$OLDIFS"
        $0 @menu@ "$modes"
		ret=$?
		test $ret -eq 255 && exit 1
        mode=$(expr $ret + 1)

        echo
		if [ -n "$prev_use_dhcp" ]; then
			echo -n "In current config, DHCP is "
			if [ "$prev_use_dhcp" == "yes" ]; then
				echo "enabled"
			else
				echo "disabled"
			fi
			TMP=" or press ENTER to keep current value"
		else
			TMP=""
		fi
        echo "Is DHCP used by your provider (MOST users should say NO)? "
        use_dhcp=""
        while [ -z "$use_dhcp" ]; do
            echo -n "(y/n$TMP) "
            read use_dhcp
			case "$use_dhcp" in
			y*|Y*)	use_dhcp="yes"
					break
					;;
			n*|N*)	use_dhcp="no"
					break
					;;
			"")		if [ -n "$prev_use_dhcp" ]; then
						use_dhcp="$prev_use_dhcp"
						echo "DHCP usage unchanged"
						break
					fi
					;;
			*)		use_dhcp=""
					;;
			esac
           	echo -e "Invalid answer, try again\n"
        done
		staticip="$prev_staticip"
		gateway="$prev_gateway"
		if [ "$use_dhcp" != "yes" ]; then
        	echo
			if [ -n "$prev_staticip" -a -n "$prev_gateway" ]; then
				echo -n "In current config, static IP is used"
			fi
        	echo "Did you get a static IP from your provider (MOST users should say NO)? "
			foo=""
        	while [ -z "$foo" ]; do
	   	        echo -n "(y/n or press ENTER to keep current setting) "
            	read foo
				case "$foo" in
				y*|Y*)	foo="yes"
						;;
				n*|N*)	foo="no"
						;;
				"")		if [ "$prev_staticip" -a -n "$prev_gateway" ]; then
							foo="yes"
						else
							foo="no"
						fi
						echo "Static IP usage unchanged"
						;;
				*)		foo=""
		            	echo -e "Invalid answer, try again\n"
						;;
				esac
        	done
			if [ "$foo" == "yes" ]; then
				TMP="(press ENTER to use $prev_staticip)"
        		echo
        		echo -n "Type in your static IP $TMP: "
				staticip=""
        		while [ -z "$staticip" ]; do
            		read staticip
					case "$staticip" in
					"")		staticip="$prev_staticip"
							echo "Static IP unchanged"
							;;
					*)		echo "$staticip" | grep -E "^([0-9]{1,3}\.){3}[0-9]{1,3}$" > /dev/null 2>&1 
			                if [ $? -ne 0 ]; then
								staticip=""
							fi
							;;
					esac
        			echo -n "Invalid static IP, try again $TMP: "
        		done
				TMP="(press ENTER to use $prev_gateway)"
        		echo
        		echo "Type in your provider's gateway IP $TMP: "
				gateway=""
        		while [ -z "$gateway" ]; do
            		read gateway
					case "$gateway" in
					"")		gateway="$prev_gateway"
							echo "Gateway unchanged"
							;;
					*)		echo "$gateway" | grep -E "^([0-9]{1,3}\.){3}[0-9]{1,3}$" > /dev/null 2>&1 
				            if [ $? -ne 0 ]; then
								gateway=""
							fi
							;;
					esac
        			echo -n "Invalid gateway IP, try again $TMP: "
        		done
			fi
		fi

		# extract values from lists
		mode="$(echo $modes | cut -f $mode -d '|')"
		modem="$(echo $modems | cut -f $modem -d '|')"
		provider="$(echo $providers | cut -f $provider -d '|')"
		# remove "previous ()" is necessary
		echo "$provider" | grep -E "^$previous \(.+\)$" > /dev/null 2>&1 && \
			provider=$(echo "$provider" | cut -d '(' -f 2 | cut -d ')' -f 1)
		echo "$modem" | grep -E "^$previous \(.+\)$" > /dev/null 2>&1 && \
			modem=$(echo "$modem" | cut -d '(' -f 2 | cut -d ')' -f 1)
		echo "$binfile" | grep -E "^$previous \(.+\)$" > /dev/null 2>&1 && \
			binfile=$(echo "$binfile" | cut -d '(' -f 2 | cut -d ')' -f 1)
		echo "$mode" | grep -E "^$previous \(.+\)$" > /dev/null 2>&1 && \
			mode=$(echo "$mode" | cut -d '(' -f 2 | cut -d ')' -f 1)
        echo
        echo "==== Configuration will be created with these values :"
        echo
        echo "  + User        : $user"
        echo "  + Password    : (hidden)"
        echo "  + Provider    : $provider"
        echo "      DNS 1     : $dns1"
        echo "      DNS 2     : $dns2"
        echo "  + VPI/VCI     : $vpi/$vci"
        echo "  + Modem       : $modem"
        echo "      VID1/PID1 : $vid1/$pid1"
        echo "      VID2/PID2 : $vid2/$pid2"
        echo "  + .bin file   : $binfile"
		echo "  + PPP mode    : $mode"
		echo "  + use DHCP    : $use_dhcp"
		if [ "$use_dhcp" != "yes" ]; then
			echo "  + static IP   : $staticip"
			echo "      gateway   : $gateway"
		fi
        echo
        echo "Press ENTER to create config files or Ctrl+C to exit now without saving."
        read quitte

        $BIN_DIR/makeconfig "$mode" "$user" "$password" "$BIN_DIR/pppoeci" $dns1 $dns2 $vpi $vci \
			"$vid1$vid2" "$vid2pid2" "$binfile" "$firmware" "$staticip" "$gateway" "$use_dhcp" \
			"$modem" "$provider"
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
        rm -rf $tmpbin > /dev/null
		test $ret -eq 255 && exit 1
        echo
        $0 $binfile
        echo
    fi
	;;

*)
    # .bin change
	$BIN_DIR/makeconfig --synch "$1"
	exit $?
	;;
esac
exit 0
