#!/bin/sh
# ****************************************************************************
# *                                                                          *
# *   eciconf - Tcl/tk GUI for ECI Linux driver configuration (makeconfig)   *
# *   v0.6      by FlashCode and Crevetor (c) 2002/04/14                     *
# *                                                                          *
# *          For any support, contact one of us :                            *
# *           - FlashCode: flashcode@free.fr  http://flashcode.free.fr       *
# *           - Crevetor : ziva@caramail.com                                 *
# *                                                                          *
# ****************************************************************************
#
# 2002/10/19, FlashCode :
#   Added Belgacom provider
# 2002/10/07, FlashCode :
#   Added modem Zyxel Prestige 630-41 and provider Bluewin
# 2002/10/05, FlashCode :
#   Added modems US Robotics 8500, BT Voyager, Xentrix USB
# 2002/06/13, FlashCode :
#   Added modem selection
# 2002/05/19, FlashCode :
#   Added .bin selection
# \
exec wish "$0" "$@" & exit 0

# <CONFIG>
set BIN_DIR "/usr/local/bin"
set ETC_DIR "/etc"
set CONF_DIR "/etc/eciadsl"
set PPPD_DIR "/etc/ppp"
# </CONFIG>

set titre_fenetre "ECI Linux driver configuration v0.6-pre4"

wm title . $titre_fenetre

#
# ===== Check is user is 'root' =====
#

catch {exec whoami} current_user
if {[string compare $current_user "root"] != 0} {
    frame .baduser
    message .baduser.texte -text "You must be root in order to run the ECI Linux driver configuration." -aspect 900 -padx 15 -pady 15
    button .baduser.ok -text { O K } -command {exit}
    pack .baduser.texte .baduser.ok -side top
    pack configure .baduser.ok -pady 15
    pack .baduser
    tkwait window .
    exit
}

#
# ===== Remove dabusb section =====
#

frame .dabusb
message .dabusb.texte -text "If your modem is running when you start Linux,\nclick here after unplugging your modem:" -aspect 600 -anchor w
button .dabusb.remove -text {Remove Dabusb} -background "#ffccff" -command {run_dabusb} -padx 10
frame .dabusb.espace -width 6
bind .dabusb.remove <Enter> {pushstate "Unplug your modem first and then click on this button to remove dabusb"}
bind .dabusb.remove <Leave> {popstate}
pack .dabusb.texte .dabusb.remove .dabusb.espace -side left

pack .dabusb -padx 10 -pady 15 -side top

#
# ===== pppoeci path, user and password =====
#

frame .frame1
label .frame1.label_chemin -text { Path to pppoeci:} -width 15 -anchor e
set path_pppoeci "$BIN_DIR/pppoeci"
entry .frame1.chemin -textvariable path_pppoeci -background lightblue -width 35
bind .frame1.chemin <Enter> {pushstate "Enter path to run pppoeci (in case of doubt, don't modify this path)"}
bind .frame1.chemin <Leave> {popstate}
pack .frame1.label_chemin .frame1.chemin -side left
pack configure .frame1.chemin -padx 15
pack .frame1 -padx 15 -pady 3

frame .frame2
label .frame2.label_user -text {User:} -anchor e
set username "username@domain"
entry .frame2.user -textvariable username -background lightblue -width 17
bind .frame2.user <Enter> {pushstate "Enter your username and domain (given by your provider)"}
bind .frame2.user <Leave> {popstate}
pack .frame2.label_user .frame2.user -side left
pack configure .frame2.user -padx 10

label .frame2.label_password -text { Password:} -anchor e
set password ""
entry .frame2.password -show * -textvariable password -background lightblue -width 13
bind .frame2.password <Enter> {pushstate "Enter your password (given by your provider)"}
bind .frame2.password <Leave> {popstate}
pack .frame2.label_password .frame2.password -side left
pack configure .frame2.password -padx 10
pack .frame2 -padx 15 -pady 6

frame .ligne_vide1 -height 15
pack .ligne_vide1

#
# ===== Internet provider DNS =====
#

frame .bloc1

frame .bloc1.espace1 -width 15

frame .bloc1.fai

frame .bloc1.fai.majdns
checkbutton .bloc1.fai.majdns.checkbox -text { Update provider DNS:} -command {invert_fai} -relief groove -background "#ffcc99" -width 28 -variable majfai -offvalue "non" -onvalue "oui" -selectcolor blue
bind .bloc1.fai.majdns.checkbox <Enter> {pushstate "Check this box if you want to update your DNS ($ETC_DIR/resolv.conf)"}
bind .bloc1.fai.majdns.checkbox <Leave> {popstate}
pack .bloc1.fai.majdns.checkbox -side left
pack .bloc1.fai.majdns  -side top -anchor w

set providers {
	"Wanadoo"			"193.252.19.3"		"193.252.19.4"
	"Club Internet"		"194.117.200.15"	"194.117.200.10"
	"9 Telecom"			"212.30.96.108"		"213.203.124.146"
	"Cegetel"			"194.6.128.3"		"194.6.128.4"
	"World Online"		"212.83.128.3"		"212.83.128.4"
	"Telecom Italia"	"212.216.112.112"	"212.216.172.62"
	"Tiscali Italia"	"195.130.224.18"	"195.130.225.129"
	"Pipex UK"			"158.43.240.4"		"158.43.240.3"
	"Bluewin"			"195.186.1.111"		"195.186.4.111"
	"Belgacom"			"195.238.2.21"		"195.238.2.22"
	"Bezeq Intl."		"192.115.106.11"	"192.115.106.10"
	"other"				""	""
}

frame .bloc1.fai.liste
listbox .bloc1.fai.liste.contenu -yscrollcommand {.bloc1.fai.liste.scroll set} -width 27 -height 7 -foreground black -selectbackground "#00ccff" -selectforeground black
.bloc1.fai.liste.contenu configure -exportselection 0
set i 0
set len [llength $providers]
while {$i<$len} {
	.bloc1.fai.liste.contenu insert end [lindex $providers $i]
	incr i 3
}

scrollbar .bloc1.fai.liste.scroll -command ".bloc1.fai.liste.contenu yview"

pack .bloc1.fai.liste.contenu .bloc1.fai.liste.scroll -side left -fill y
pack .bloc1.fai.liste

frame .bloc1.fai.espacevertic -height 5
pack .bloc1.fai.espacevertic

frame .bloc1.fai.dns1
label .bloc1.fai.dns1.label -text "DNS 1: " -width 8
entry .bloc1.fai.dns1.entry -textvariable dns1 -background lightblue -width 15
pack .bloc1.fai.dns1.label .bloc1.fai.dns1.entry -side left
pack .bloc1.fai.dns1

frame .bloc1.fai.dns2
label .bloc1.fai.dns2.label -text "DNS 2: " -width 8
entry .bloc1.fai.dns2.entry -textvariable dns2 -background lightblue -width 15
pack .bloc1.fai.dns2.label .bloc1.fai.dns2.entry -side left -anchor e
pack .bloc1.fai.dns2

.bloc1.fai.liste.contenu configure -foreground darkgray -selectbackground lightgray -selectforeground darkgray
.bloc1.fai.dns1.entry configure -state disabled -foreground darkgray -background lightgray
.bloc1.fai.dns2.entry configure -state disabled -foreground darkgray -background lightgray
.bloc1.fai.dns1.label configure -foreground darkgray
.bloc1.fai.dns2.label configure -foreground darkgray
set majfai "non"
set fai_choisi ""

bind .bloc1.fai.liste.contenu <ButtonRelease> {select_provider}
bind .bloc1.fai.liste.contenu <Enter> {pushstate "Select a provider"}
bind .bloc1.fai.liste.contenu <Leave> {popstate}
bind .bloc1.fai.dns1 <Enter> {pushstate "\[OPTIONAL\] Enter your own primary DNS (given by your provider)"}
bind .bloc1.fai.dns1 <Leave> {popstate}
bind .bloc1.fai.dns2 <Enter> {pushstate "\[OPTIONAL\] Enter your own secondary DNS (given by your provider)"}
bind .bloc1.fai.dns2 <Leave> {popstate}

set dns1 ""
set dns2 ""

frame .bloc1.espace2 -width 15

#
# ===== Modem selection =====
#

frame .bloc1.modem

label .bloc1.modem.libelle -text "Select your modem:" -relief groove -background "#ffcc99" -width 46
pack .bloc1.modem.libelle

set modems {
			"Archtek UGW-8000"					"0915"	"0001"	"0915"	"0002"
			"Askey USB-ADSL"					"1690"	"0205"	"0915"	"0206"
			"Aztech 100U"						"0509"	"0801"	"0915"	"0802"
			"BT Voyager"						"1690"	"0203"	"0915"	"0204"
			"Cypress Globespan ADSL USB G7000"	"0915"	"0001"	"0915"	"0002"
			"Digicom MichelAngelo"				"0547"	"2131"	"0915"	"8000"
			"ECI HiFocus/B-Focus"				"0547"	"2131"	"0915"	"8000"
			"Eicon Diva"						"071d"	"ac81"	"0915"	"ac82"
			"Ericsson hm120dp"					"08ea"	"00c9"	"0915"	"00ca"
			"Fujitsu FDX310"					"0e60"	"0101"	"0915"	"0102"
			"GVC BB039"							"0915"	"0001"	"0915"	"0002"
			"Siemens Santis"					"0915"	"0001"	"0915"	"0002"
			"Topcom Webr@cer 850"				"0915"	"0001"	"0915"	"0002"
			"US Robotics 8500"					"0baf"	"00e6"	"0915"	"00e7"
			"Webpower Ipmdatacom"				"0915"	"0001"	"0915"	"0002"
			"Wisecom ad80usg or EA100"			"0915"	"0001"	"0915"	"0002"
			"Xentrix USB"						"0e60"	"0100"	"0915"	"0101"
			"Zyxel Prestige 630-41"				"0547"	"2131"	"0915"	"8000"
			"other"								""		""		""		""
}

frame .bloc1.modem.liste
listbox .bloc1.modem.liste.contenu -yscrollcommand {.bloc1.modem.liste.scroll set} -width 43 -height 7 -foreground black -selectbackground "#00ccff" -selectforeground black
.bloc1.modem.liste.contenu configure -exportselection 0

set i 0
set len [llength $modems]
while {$i<$len} {
	.bloc1.modem.liste.contenu insert end [lindex $modems $i]
	incr i 5
}

scrollbar .bloc1.modem.liste.scroll -command ".bloc1.modem.liste.contenu yview"

pack .bloc1.modem.liste.contenu .bloc1.modem.liste.scroll -side left -fill y
pack .bloc1.modem.liste

frame .bloc1.modem.espacevertic -height 5
pack .bloc1.modem.espacevertic

frame .bloc1.modem.vidpid1
label .bloc1.modem.vidpid1.labelvid -text "VID 1: " -width 8
entry .bloc1.modem.vidpid1.entryvid -textvariable vid1 -background lightblue -width 7
pack .bloc1.modem.vidpid1.labelvid .bloc1.modem.vidpid1.entryvid -side left
label .bloc1.modem.vidpid1.labelpid -text "PID 1: " -width 8
entry .bloc1.modem.vidpid1.entrypid -textvariable pid1 -background lightblue -width 7
pack .bloc1.modem.vidpid1.labelpid .bloc1.modem.vidpid1.entrypid -side left
pack .bloc1.modem.vidpid1

frame .bloc1.modem.vidpid2
label .bloc1.modem.vidpid2.labelvid -text "VID 2: " -width 8
entry .bloc1.modem.vidpid2.entryvid -textvariable vid2 -background lightblue -width 7
pack .bloc1.modem.vidpid2.labelvid .bloc1.modem.vidpid2.entryvid -side left
label .bloc1.modem.vidpid2.labelpid -text "PID 2: " -width 8
entry .bloc1.modem.vidpid2.entrypid -textvariable pid2 -background lightblue -width 7
pack .bloc1.modem.vidpid2.labelpid .bloc1.modem.vidpid2.entrypid -side left
pack .bloc1.modem.vidpid2

.bloc1.modem.liste.contenu configure -foreground black -selectbackground "#00ccff" -selectforeground black
.bloc1.modem.vidpid1.entryvid configure -state normal -foreground darkgray -background lightgray
.bloc1.modem.vidpid1.entrypid configure -state normal -foreground darkgray -background lightgray
.bloc1.modem.vidpid2.entryvid configure -state normal -foreground darkgray -background lightgray
.bloc1.modem.vidpid2.entrypid configure -state normal -foreground darkgray -background lightgray
.bloc1.modem.vidpid1.labelvid configure -foreground darkgray
.bloc1.modem.vidpid1.labelpid configure -foreground darkgray
.bloc1.modem.vidpid2.labelvid configure -foreground darkgray
.bloc1.modem.vidpid2.labelpid configure -foreground darkgray

bind .bloc1.modem.liste.contenu <ButtonRelease> {select_modem}
bind .bloc1.modem.liste.contenu <Enter> {pushstate "Select a modem"}
bind .bloc1.modem.liste.contenu <Leave> {popstate}
bind .bloc1.modem.vidpid1 <Enter> {pushstate "These are the vendor ID and product ID of your modem (4-digit hexa)"}
bind .bloc1.modem.vidpid1 <Leave> {popstate}
bind .bloc1.modem.vidpid2 <Enter> {pushstate "These are the vendor ID and product ID of your modem once initialized (4-digit hexa)"}
bind .bloc1.modem.vidpid2 <Leave> {popstate}

frame .bloc1.espace3 -width 15

pack .bloc1.espace1 .bloc1.fai .bloc1.espace2 .bloc1.modem .bloc1.espace3 -side left -anchor n

pack .bloc1

frame .ligne_vide2 -height 20
pack .ligne_vide2

#
# ===== List of .bin =====
#

frame .bloc2

frame .bloc2.listebin

checkbutton .bloc2.listebin.checkbox -text { Change synch .bin file (only if driver hangs up):} -command {invert_bin} -relief groove -background "#ffcc99" -width 45 -variable majbin -offvalue "non" -onvalue "oui" -selectcolor blue
bind .bloc2.listebin.checkbox <Enter> {pushstate "Check this box if you want to change your synch .bin file"}
bind .bloc2.listebin.checkbox <Leave> {popstate}
set majbin "non"

set lien_bin_final "$CONF_DIR/synch.bin"
set nom_bin_actuel [exec ls -l "$lien_bin_final" | sed "s/.*->\ //" ]

label .bloc2.listebin.actuel -text "Current .bin: $nom_bin_actuel" -relief sunken -width 48 -anchor w

frame .bloc2.listebin.liste
listbox .bloc2.listebin.liste.contenu -yscrollcommand ".bloc2.listebin.liste.scroll set" -width 45 -height 4 -foreground darkgray -selectbackground lightgray -selectforeground darkgray
.bloc2.listebin.liste.contenu configure -exportselection 0

proc add_bins {chemin} {
	global .bloc2.listebin nom_bin_actuel lien_bin_final

    set returncode [catch {exec find $chemin -name "*.bin" } bin_trouves]
    if {$returncode != 0} {
    } else {
        foreach bin $bin_trouves {
            if {$lien_bin_final != $bin && ![regexp "firmware" $bin] && [lsearch -glob [.bloc2.listebin.liste.contenu get 0 end] $bin] == -1} {
                if {[string compare $bin $nom_bin_actuel] != 0} {
                    .bloc2.listebin.liste.contenu insert end $bin
                }
            }
        }
    }
}

add_bins "$CONF_DIR"

.bloc2.listebin.liste.contenu selection set 0
set bin_choisi [.bloc2.listebin.liste.contenu curselection]
set bin_initial $bin_choisi
bind .bloc2.listebin.liste.contenu <Enter> {pushstate "Choose another .bin (ONLY if driver hangs up into eci-load2 !)"}
bind .bloc2.listebin.liste.contenu <Leave> {popstate}
scrollbar .bloc2.listebin.liste.scroll -command ".bloc2.listebin.liste.contenu yview"
pack .bloc2.listebin.liste.contenu .bloc2.listebin.liste.scroll -side left -fill y

frame .bloc2.listebin.recherche
label .bloc2.listebin.recherche.texte -text {Search .bin here:} -width 15
set chemin_bin $CONF_DIR
entry .bloc2.listebin.recherche.chemin -textvariable chemin_bin -background "#CCEEEE" -width 27
bind .bloc2.listebin.recherche.chemin <Enter> {pushstate "Enter path for searching .bin files"}
bind .bloc2.listebin.recherche.chemin <Leave> {popstate}
image create bitmap bitmaptest -data "\
#define loupe_width 25 \
#define loupe_height 15 \
static unsigned char loupe_bits[] = { \
   0x00, 0xe0, 0x01, 0x00, 0x00, 0x10, 0x02, 0x00, 0x00, 0x08, 0x04, 0x00, \
   0x00, 0x04, 0x08, 0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x04, 0x08, 0x00, \
   0x00, 0x04, 0x08, 0x00, 0x00, 0x08, 0x04, 0x00, 0x00, 0x1c, 0x02, 0x00, \
   0x00, 0xee, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, \
   0xc0, 0x01, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00 };"
button .bloc2.listebin.recherche.search -image bitmaptest -background "#99CCCC" -command {add_bins $chemin_bin}
bind .bloc2.listebin.recherche.search <Enter> {pushstate "Click to search .bin files (warning: can take a moment!)"}
bind .bloc2.listebin.recherche.search <Leave> {popstate}
pack .bloc2.listebin.recherche.texte .bloc2.listebin.recherche.chemin .bloc2.listebin.recherche.search -side left

pack .bloc2.listebin.checkbox
pack .bloc2.listebin.actuel
pack .bloc2.listebin.liste
pack .bloc2.listebin.recherche
pack .bloc2.listebin -padx 15

frame .bloc2.espace1 -width 15

#
# ===== VPI/VCI =====
#

frame .bloc2.vpci

label .bloc2.vpci.libelle -text "Your VPI/VCI:" -relief groove -background "#ffcc99" -width 15

frame .bloc2.vpci.vpi_espace -height 5
frame .bloc2.vpci.vpi
label .bloc2.vpci.vpi.label -text "VPI:"
set vpi "8"
entry .bloc2.vpci.vpi.entry -textvariable vpi -background lightblue -width 4
bind .bloc2.vpci.vpi.entry <Enter> {pushstate "VPI given by your provider (8 for France)"}
bind .bloc2.vpci.vpi.entry <Leave> {popstate}

frame .bloc2.vpci.vci_espace -height 10
frame .bloc2.vpci.vci
label .bloc2.vpci.vci.label -text "VCI:"
set vci "35"
entry .bloc2.vpci.vci.entry -textvariable vci -background lightblue -width 4
bind .bloc2.vpci.vci.entry <Enter> {pushstate "VCI given by your provider (35 for France)"}
bind .bloc2.vpci.vci.entry <Leave> {popstate}

pack .bloc2.vpci.libelle -side top
pack .bloc2.vpci.vpi.label .bloc2.vpci.vpi.entry -side left -padx 5
pack .bloc2.vpci.vci.label .bloc2.vpci.vci.entry -side left -padx 5

# Modem image :

frame .bloc2.vpci.espace -height 3
image create photo modem_eci -file "$CONF_DIR/modemeci.gif"
label .bloc2.vpci.image -image modem_eci
bind .bloc2.vpci.image <Enter> {pushstate "ECI HiFocus USB ADSL modem"}
bind .bloc2.vpci.image <Leave> {popstate}

pack .bloc2.vpci.libelle .bloc2.vpci.vpi_espace .bloc2.vpci.vpi .bloc2.vpci.vci .bloc2.vpci.espace .bloc2.vpci.image -side top

frame .bloc2.espace2 -width 20

pack .bloc2.listebin .bloc2.espace1 .bloc2.vpci .bloc2.espace2 -side left -anchor n

pack .bloc2

frame .ligne_vide3 -height 15
pack .ligne_vide3

#
# ===== OK and Cancel buttons =====
#

frame .boutons
button .boutons.create -text {Create config !} -width 15 -height 1 -command {run_makeconfig "$username" "$password" "$path_pppoeci" "$dns1" "$dns2" $vpi $vci $vid1 $pid1 $vid2 $pid2} -state disabled
bind .boutons.create <Enter> {pushstate "Save modifications then quit: select a modem first"}
bind .boutons.create <Leave> {popstate}
frame .boutons.espace -width 20
button .boutons.set_bin -text {Change synch .bin} -width 15 -height 1 -command {run_eciconftxt} -state disabled
bind .boutons.set_bin <Enter> {pushstate "Only change current synch .bin: enable it above and select a .bin"}
bind .boutons.set_bin <Leave> {popstate}
frame .boutons.espace2 -width 20
button .boutons.cancel -text {Cancel} -background "#ffbbbb" -width 15 -height 1 -command {exit}
bind .boutons.cancel <Enter> {pushstate "Quit without saving"}
bind .boutons.cancel <Leave> {popstate}
pack .boutons.create .boutons.espace .boutons.set_bin .boutons.espace2 .boutons.cancel -side left
pack .boutons

frame .ligne_vide4 -height 15
pack .ligne_vide4

label .state -textvariable statetext -borderwidth 2 -relief sunken -anchor w
pack .state -fill x
set statetext "Ready."

#
# ===== call to makeconfig =====
#

proc conf_report {exit msg color sortie} {
	global titre_fenetre

    if {"$exit"=="oui"} {
		catch {destroy .confok}
	}
	toplevel .confok
    wm title .confok $titre_fenetre
    message .confok.texte -text "$msg" -aspect 600 -padx 15 -pady 15
	
    if {"$sortie"!=""} {
	    message .confok.sortie -text "$sortie" -aspect 600 -padx 15 -pady 15 -background $color
	} else {
	    message .confok.sortie
	}

    if {"$exit"=="oui"} {
	    button .confok.ok -text { Quit } -command {exit}
	} else {
	    button .confok.ok -text { Close } -command {destroy .confok}
	}

    pack .confok.texte .confok.sortie .confok.ok -side top
    pack configure .confok.ok -pady 15
    grab set .confok
    tkwait window .confok
}

proc run_eciconftxt {} {
	global BIN_DIR majbin

    if {[string compare $majbin "oui"] == 0} {
        set numero_bin_choix [.bloc2.listebin.liste.contenu curselection]
        set nom_bin_choix [.bloc2.listebin.liste.contenu get $numero_bin_choix]
	    set returncode [catch {exec $BIN_DIR/eciconftxt.sh "$nom_bin_choix"} sortie]
	    if {$returncode != 0} {
	        conf_report "non" "Synch .bin has NOT been set.\n\nThis is the error:" "#ffbbbb" $sortie
	    } else {
	        conf_report "non" "Synch .bin has been changed and will\nbe used the next time you run startmodem." lightgreen ""
	    }
	}
}

proc run_makeconfig {username password path_pppoeci dns1 dns2 vpi vci vid1 pid1 vid2 pid2} {
	global majfai majbin BIN_DIR

    if {[string compare $majfai "oui"] == 0} {
        set srvdns1 $dns1
        set srvdns2 $dns2
    } else {
        set srvdns1 0
        set srvdns2 0
    }
    if {[string compare $majbin "oui"] == 0} {
        set numero_bin_choix [.bloc2.listebin.liste.contenu curselection]
        set nom_bin_choix [.bloc2.listebin.liste.contenu get $numero_bin_choix]
        set returncode [catch {exec $BIN_DIR/makeconfig "$username" "$password" "$path_pppoeci" "$srvdns1" "$srvdns2" $vpi $vci $vid1$pid1 $vid2$pid2 "$nom_bin_choix"} sortie]
    } else {
        set returncode [catch {exec $BIN_DIR/makeconfig "$username" "$password" "$path_pppoeci" "$srvdns1" "$srvdns2" $vpi $vci $vid1$pid1 $vid2$pid2} sortie]
    }
    if {$returncode != 0} {
        conf_report "oui" "Makeconfig did not update your files.\n\nThis is the error:" "#ffbbbb" $sortie
    } else {
        conf_report "oui" "Configuration files updated with success!\n\nThis is makeconfig output:" lightgreen $sortie
    }
}

#
# ===== call to remove_dabusb =====
#

proc run_dabusb {} {
    set returncode [catch {exec remove_dabusb} sortie]
    toplevel .dab
    wm title .dab "dabusb"
    if {$returncode != 0} {
        set msg "Dabusb couldn't be removed...\n\nThe error is:"
        set vmtitle "Error"
        set color "#ffbbbb"
    } else {
        set msg "Dabusb succesfully removed.\n\nThis is dabusb output:"
        set vmtitle "OK"
        set color lightgreen
    }
    message .dab.texte -text "$msg" -aspect 600 -padx 15 -pady 15
    message .dab.sortie -text "$sortie" -aspect 600 -padx 15 -pady 15 -background $color
    button .dab.ok -text { O K } -command {destroy .dab}
    pack .dab.texte .dab.sortie .dab.ok -side top
    pack configure .dab.ok -pady 15
    grab set .dab
    tkwait window .dab
}

#
# ===== display field help =====
#

proc pushstate {msg} {
	global statetext statepile topstatepile

    if {[info exists topstatepile] == 0} {
        set topstatepile 0
    }
    incr topstatepile 1
    set statepile{$topstatepile} $msg
    set statetext $msg
}

proc popstate {} {
	global statetext statepile topstatepile

    if {[info exists topstatepile] != 0} {
        if {$topstatepile > 1} {
            unset statepile($topstatepile)
            incr topstatepile -1
            set statetext $statepile($topstatepile)
        } else {
            set topstatepile 0
            set statetext "Ready."
        }
    }
}

#
# ===== invert fai state (enabled/disabled) =====
#

proc invert_fai {} {
	global majfai fai_choisi providers

    if {"$majfai"=="oui"} {
    	.bloc1.fai.liste.contenu configure -foreground black -selectbackground "#00ccff" -selectforeground black
		.bloc1.fai.liste.contenu selection clear 0 [expr [llength $providers]-1]
		if {$fai_choisi!=""} {
	    	.bloc1.fai.dns1.entry configure -state normal -foreground black -background lightblue
			.bloc1.fai.dns2.entry configure -state normal -foreground black -background lightblue
	    	.bloc1.fai.dns1.label configure -foreground black
	    	.bloc1.fai.dns2.label configure -foreground black
			.bloc1.fai.liste.contenu selection set $fai_choisi
		} else {
			.bloc1.fai.liste.contenu selection set 0
		}
		select_provider
    } else {
        set fai_choisi [.bloc1.fai.liste.contenu curselection]
        .bloc1.fai.liste.contenu configure -foreground darkgray -selectbackground lightgray -selectforeground darkgray
        .bloc1.fai.dns1.entry configure -state disabled -foreground darkgray -background lightgray
        .bloc1.fai.dns2.entry configure -state disabled -foreground darkgray -background lightgray
        .bloc1.fai.dns1.label configure -foreground darkgray
        .bloc1.fai.dns2.label configure -foreground darkgray
    }
}

#
# ===== invert .bin files (enabled/disabled) =====
#

proc invert_bin {} {
	global majbin bin_choisi bin_initial
	global .boutons.set_bin

    if {[string compare $majbin "oui"] == 0} {
        if {[.bloc2.listebin.liste.contenu size] > 0} {
            if {[string compare $bin_choisi ""] == 0} {
                .bloc2.listebin.liste.contenu selection set 0
                set bin_choisi [.bloc2.listebin.liste.contenu curselection]
            }
            .bloc2.listebin.liste.contenu configure -foreground black -selectbackground "#00ccff" -selectforeground black
            .bloc2.listebin.liste.contenu selection clear 0 end
            .bloc2.listebin.liste.contenu selection set $bin_choisi
            .bloc2.listebin.liste.contenu see $bin_choisi
			.boutons.set_bin configure -state normal -background lightblue
			bind .boutons.set_bin <Enter> {pushstate "Only change current synch .bin with the selected one"}
			bind .boutons.set_bin <Leave> {popstate}
        } else {
            set majbin "non"
        }
    } else {
        .bloc2.listebin.liste.contenu configure -foreground darkgray -selectbackground lightgray -selectforeground darkgray
        set bin_choisi [.bloc2.listebin.liste.contenu curselection]
        if {[string compare $bin_choisi ""] == 0} {
            set bin_choisi $bin_initial
        }
		.boutons.set_bin configure -state disabled
		bind .boutons.set_bin <Enter> {pushstate "Only change current synch .bin: enable it above and select a .bin"}
		bind .boutons.set_bin <Leave> {popstate}
    }
}

proc select_modem {} {
	global .bloc1.modem.liste.contenu .boutons.create
	global .bloc1.modem.vidpid1.entryvid .bloc1.modem.vidpid1.entrypid
	global .bloc1.modem.vidpid2.entryvid .bloc1.modem.vidpid2.entrypid
	global .bloc1.modem.vidpid1.labelvid .bloc1.modem.vidpid1.labelpid
	global .bloc1.modem.vidpid2.labelvid .bloc1.modem.vidpid2.labelpid
	global modems vid1 pid1 vid2 pid2

	set index [.bloc1.modem.liste.contenu curselection]
	if {"$index"!=""} {
		set vid1 [lindex $modems [expr $index*5+1]]
		set pid1 [lindex $modems [expr $index*5+2]]
		set vid2 [lindex $modems [expr $index*5+3]]
		set pid2 [lindex $modems [expr $index*5+4]]
		.bloc1.modem.vidpid1.entryvid configure -state normal -foreground black -background lightblue
		.bloc1.modem.vidpid1.entrypid configure -state normal -foreground black -background lightblue
		.bloc1.modem.vidpid2.entryvid configure -state normal -foreground black -background lightblue
		.bloc1.modem.vidpid2.entrypid configure -state normal -foreground black -background lightblue
		.bloc1.modem.vidpid1.labelvid configure -foreground black
		.bloc1.modem.vidpid1.labelpid configure -foreground black
		.bloc1.modem.vidpid2.labelvid configure -foreground black
		.bloc1.modem.vidpid2.labelpid configure -foreground black
		.boutons.create configure -state normal -background lightgreen
		bind .boutons.create <Enter> {pushstate "Save modifications: write configuration to files and set synch .bin if enabled, then quit"}
		bind .boutons.create <Leave> {popstate}
	}
}

proc select_provider {} {
	global .bloc1.fai.liste.contenu
	global providers dns1 dns2 majfai

	set index [.bloc1.fai.liste.contenu curselection]
	if {"$index"!=""} {
		set dns1 [lindex $providers [expr $index*3+1]]
		set dns2 [lindex $providers [expr $index*3+2]]
		if {"$majfai"=="oui"} {
			.bloc1.fai.dns1.entry configure -state normal -foreground black -background lightblue
			.bloc1.fai.dns2.entry configure -state normal -foreground black -background lightblue
			.bloc1.fai.dns1.label configure -foreground black
			.bloc1.fai.dns2.label configure -foreground black
		}
	}
}
